#include "PipelineExecutor.h"

#include "../FullscreenQuad.h"
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

void applyBindBuffer(const PipelineCommand& command, const PipelineResources& resources, Shader& shader, int& maxBoundUnit)
{
    const Texture2D* texture = findResource(resources.textures, command.sourceRT, "texture");
    const int unit = command.bufIndex;

    shader.setInt(command.sampler.c_str(), unit);
    texture->bind(static_cast<unsigned int>(unit));
    maxBoundUnit = std::max(maxBoundUnit, unit);

    std::cout << "[XML] BindBuffer " << command.sampler << "/" << command.sourceRT
              << " unit " << unit
              << " -> glUniform1i + glActiveTexture + glBindTexture\n";
}
}

void PipelineExecutor::executeStage(
    const PipelineStage& stage,
    const PipelineResources& resources) const
{
    if (!stage.enabled)
    {
        return;
    }

    if (resources.quad == nullptr)
    {
        throw std::runtime_error("Pipeline missing fullscreen quad.");
    }

    std::vector<const PipelineCommand*> pendingBindBuffers;
    int maxBoundUnit = -1;

    for (const PipelineCommand& command : stage.commands)
    {
        switch (command.type)
        {
        case PipelineCommandType::SwitchTarget:
        {
            Framebuffer* target = findResource(resources.renderTargets, command.target, "render target");
            target->bind();
            glClearColor(0.02f, 0.03f, 0.04f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            std::cout << "[XML] SwitchTarget " << command.target
                      << " -> glBindFramebuffer + glViewport\n";
            break;
        }
        case PipelineCommandType::BindBuffer:
            pendingBindBuffers.push_back(&command);
            break;
        case PipelineCommandType::DrawQuad:
        {
            Shader* shader = findResource(resources.shaders, command.context, "shader");
            shader->use();

            for (const PipelineCommand* bindBuffer : pendingBindBuffers)
            {
                applyBindBuffer(*bindBuffer, resources, *shader, maxBoundUnit);
            }

            pendingBindBuffers.clear();
            resources.quad->draw();

            std::cout << "[XML] DrawQuad context " << command.context
                      << " -> glUseProgram + glDrawArrays\n";
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
