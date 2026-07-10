#include "PipelineExecutor.h"

#include "../FullscreenQuad.h"
#include "../Material/Material.h"
#include "../Material/MaterialManager.h"
#include "../Renderer/Framebuffer.h"
#include "../Renderer/Texture2D.h"
#include "../Shader.h"

#include <glad/glad.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{
template <typename Map>
auto findResource(const Map& resources, const std::string& id, const char* resourceType)
{
    const auto found = resources.find(id);
    if (found == resources.end() || found->second == nullptr)
    {
        throw std::runtime_error(std::string("Pipeline missing ") + resourceType + ": " + id);
    }

    return found->second;
}

void bindTextureId(unsigned int textureId, unsigned int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void validateTextureUnit(int unit)
{
    int maxUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
    if (unit < 0 || unit >= maxUnits)
    {
        throw std::runtime_error("Pipeline texture unit out of range: " + std::to_string(unit));
    }
}

unsigned int renderTargetTextureId(const Framebuffer& renderTarget)
{
    if (renderTarget.textureId() != 0)
    {
        return renderTarget.textureId();
    }

    if (renderTarget.depthTextureId() != 0)
    {
        return renderTarget.depthTextureId();
    }

    throw std::runtime_error("Pipeline render target has no bindable texture.");
}

void applyBindBuffer(const PipelineCommand& command, const RendererResources& resources, Shader& shader, int& maxBoundUnit)
{
    const int unit = command.bufIndex;
    validateTextureUnit(unit);

    shader.setInt(command.sampler.c_str(), unit);

    const auto texture = resources.textures.find(command.sourceRT);
    if (texture != resources.textures.end() && texture->second != nullptr)
    {
        texture->second->bind(static_cast<unsigned int>(unit));
        std::cout << "[XML] BindBuffer " << command.sampler << "/" << command.sourceRT
                  << " unit " << unit
                  << " -> glUniform1i + glActiveTexture + glBindTexture\n";
    }
    else
    {
        const Framebuffer* renderTarget = findResource(resources.renderTargets, command.sourceRT, "render target texture");
        bindTextureId(renderTargetTextureId(*renderTarget), static_cast<unsigned int>(unit));
        std::cout << "[XML] BindBuffer " << command.sampler << "/" << command.sourceRT
                  << " unit " << unit
                  << " -> bind render target texture\n";
    }

    maxBoundUnit = std::max(maxBoundUnit, unit);
}

void clearTarget(const PipelineCommand& command)
{
    GLbitfield clearMask = 0;

    if (command.clearDepth)
    {
        glClearDepth(command.clearDepthValue);
        clearMask |= GL_DEPTH_BUFFER_BIT;
    }

    if (command.clearColor0)
    {
        glClearColor(command.clearR, command.clearG, command.clearB, command.clearA);
        clearMask |= GL_COLOR_BUFFER_BIT;
    }

    if (clearMask != 0)
    {
        glClear(clearMask);
    }

    std::cout << "[XML] ClearTarget color=" << (command.clearColor0 ? "true" : "false")
              << " depth=" << (command.clearDepth ? "true" : "false")
              << " -> glClearColor/glClearDepth/glClear\n";
}

void drawGeometry(
    const PipelineCommand& command,
    const RendererResources& resources,
    std::vector<const PipelineCommand*>& pendingBindBuffers,
    int& maxBoundUnit)
{
    const auto shader = resources.shaders.find(command.context);
    if (shader != resources.shaders.end() && shader->second != nullptr)
    {
        shader->second->use();

        for (const PipelineCommand* bindBuffer : pendingBindBuffers)
        {
            applyBindBuffer(*bindBuffer, resources, *shader->second, maxBoundUnit);
        }

        pendingBindBuffers.clear();
    }

    const auto draw = resources.geometryDraws.find(command.context);
    if (draw != resources.geometryDraws.end())
    {
        draw->second();
        std::cout << "[XML] DrawGeometry context " << command.context
                  << " -> geometry callback -> glDrawElements\n";
    }
    else
    {
        std::cout << "[XML] DrawGeometry context " << command.context
                  << " -> no registered callback\n";
    }
}

bool applyShaderSetup(const std::string& context, const RendererResources& resources, Shader& shader)
{
    const auto setup = resources.shaderSetups.find(context);
    if (setup == resources.shaderSetups.end())
    {
        return false;
    }

    setup->second(shader);
    return true;
}
}

void PipelineExecutor::executeStage(
    const PipelineStage& stage,
    const RendererResources& resources) const
{
    if (!stage.enabled)
    {
        return;
    }

    std::vector<const PipelineCommand*> pendingBindBuffers;
    int maxBoundUnit = -1;

    for (const PipelineCommand& command : stage.commands)
    {
        switch (command.type)
        {
        case PipelineCommandType::SwitchTarget:
        {
            if (command.target.empty())
            {
                Framebuffer::unbind();
                std::cout << "[XML] SwitchTarget default -> glBindFramebuffer\n";
            }
            else
            {
                Framebuffer* target = findResource(resources.renderTargets, command.target, "render target");
                target->bind();
                std::cout << "[XML] SwitchTarget " << command.target
                          << " -> glBindFramebuffer + glViewport\n";
            }
            break;
        }
        case PipelineCommandType::ClearTarget:
            clearTarget(command);
            break;
        case PipelineCommandType::BindBuffer:
            pendingBindBuffers.push_back(&command);
            break;
        case PipelineCommandType::DrawGeometry:
            drawGeometry(command, resources, pendingBindBuffers, maxBoundUnit);
            break;
        case PipelineCommandType::DrawQuad:
        {
            if (command.material.empty())
            {
                throw std::runtime_error("Pipeline DrawQuad is missing material path.");
            }

            if (command.context.empty())
            {
                throw std::runtime_error("Pipeline DrawQuad is missing context for material: " + command.material);
            }

            if (resources.materialManager == nullptr)
            {
                throw std::runtime_error("Pipeline DrawQuad is missing a material manager.");
            }

            if (resources.assetRoot.empty())
            {
                throw std::runtime_error("Pipeline DrawQuad is missing an asset root for material: " + command.material);
            }

            const std::filesystem::path materialPath = resources.assetRoot / command.material;
            Material& material = resources.materialManager->load(materialPath);
            Shader& shader = material.shader();
            shader.use();
            const bool setupApplied = applyShaderSetup(command.context, resources, shader);

            for (const PipelineCommand* bindBuffer : pendingBindBuffers)
            {
                applyBindBuffer(*bindBuffer, resources, shader, maxBoundUnit);
            }

            pendingBindBuffers.clear();
            if (resources.quad == nullptr)
            {
                throw std::runtime_error("Pipeline missing fullscreen quad.");
            }

            resources.quad->draw();

            std::cout << "[XML] DrawQuad material " << command.material
                      << " context " << command.context
                      << " -> Material -> Shader -> glUseProgram"
                      << (setupApplied ? " + setup uniforms" : "")
                      << " + glDrawArrays\n";
            break;
        }
        case PipelineCommandType::UnbindBuffers:
            for (int unit = 0; unit <= maxBoundUnit; ++unit)
            {
                Texture2D::unbind(static_cast<unsigned int>(unit));
            }

            glActiveTexture(GL_TEXTURE0);
            std::cout << "[XML] UnbindBuffers -> glBindTexture(0)\n";
            break;
        }
    }
}
