#include "App.h"

#include "FullscreenQuad.h"
#include "Pipeline/PipelineExecutor.h"
#include "Pipeline/PipelineParser.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Texture2D.h"
#include "Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr int WindowWidth = 1024;
constexpr int WindowHeight = 1024;
constexpr int ProceduralTextureSize = 256;
constexpr float Pi = 3.14159265358979323846f;

std::string shaderPath(const char* fileName)
{
    return std::string(ASSET_ROOT) + "/shaders/" + fileName;
}

std::string pipelinePath(const char* fileName)
{
    return std::string(ASSET_ROOT) + "/pipelines/" + fileName;
}

const char* commandTypeName(PipelineCommandType type)
{
    switch (type)
    {
    case PipelineCommandType::SwitchTarget:
        return "SwitchTarget";
    case PipelineCommandType::BindBuffer:
        return "BindBuffer";
    case PipelineCommandType::DrawQuad:
        return "DrawQuad";
    case PipelineCommandType::UnbindBuffers:
        return "UnbindBuffers";
    }

    return "Unknown";
}

void printPipelineCommands(const std::vector<PipelineStage>& stages)
{
    std::cout << "Parsed pipeline stages:\n";

    for (const PipelineStage& stage : stages)
    {
        std::cout << "  Stage id=\"" << stage.id << "\" enabled=" << (stage.enabled ? "true" : "false") << '\n';

        for (const PipelineCommand& command : stage.commands)
        {
            std::cout << "    " << commandTypeName(command.type);

            if (!command.target.empty())
            {
                std::cout << " target=\"" << command.target << "\"";
            }

            if (!command.sampler.empty())
            {
                std::cout << " sampler=\"" << command.sampler << "\"";
            }

            if (!command.sourceRT.empty())
            {
                std::cout << " sourceRT=\"" << command.sourceRT << "\"";
            }

            if (command.type == PipelineCommandType::BindBuffer)
            {
                std::cout << " bufIndex=" << command.bufIndex;
            }

            if (!command.material.empty())
            {
                std::cout << " material=\"" << command.material << "\"";
            }

            if (!command.context.empty())
            {
                std::cout << " context=\"" << command.context << "\"";
            }

            std::cout << '\n';
        }
    }
}

const PipelineStage& findPipelineStage(const std::vector<PipelineStage>& stages, const std::string& id)
{
    const auto found = std::find_if(
        stages.begin(),
        stages.end(),
        [&id](const PipelineStage& stage)
        {
            return stage.id == id;
        });

    if (found == stages.end())
    {
        throw std::runtime_error("Pipeline stage not found: " + id);
    }

    return *found;
}

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

float normalizedCoord(int value, int size)
{
    return (static_cast<float>(value) + 0.5f) / static_cast<float>(size);
}

std::vector<float> createDensityTexture(int width, int height)
{
    std::vector<float> data(width * height);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const float u = normalizedCoord(x, width);
            const float v = normalizedCoord(y, height);
            const float dx = u - 0.5f;
            const float dy = v - 0.5f;
            const float distance = std::sqrt(dx * dx + dy * dy);
            data[y * width + x] = std::clamp(1.0f - distance / 0.52f, 0.0f, 1.0f);
        }
    }

    return data;
}

std::vector<float> createNoiseTexture(int width, int height)
{
    std::vector<float> data(width * height);
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    for (float& value : data)
    {
        value = distribution(rng);
    }

    return data;
}

std::vector<float> createMetalTexture(int width, int height)
{
    std::vector<float> data(width * height, 0.0f);
    const float lineCenter = 0.64f;
    const float lineWidth = 0.008f;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const float u = normalizedCoord(x, width);
            const float distance = std::abs(u - lineCenter);
            data[y * width + x] = distance < lineWidth ? 1.0f : 0.0f;
        }
    }

    return data;
}

std::vector<float> createLungsTexture(int width, int height)
{
    std::vector<float> data(width * height, 1.0f);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const float u = normalizedCoord(x, width);
            const float v = normalizedCoord(y, height);
            const float dx = u - 0.5f;
            const float dy = v - 0.58f;
            const float distance = std::sqrt(dx * dx + dy * dy);
            const float darkRegion = 1.0f - std::clamp((distance - 0.22f) / 0.04f, 0.0f, 1.0f);
            data[y * width + x] = 1.0f - darkRegion * 0.92f;
        }
    }

    return data;
}

std::vector<float> createEchoMaskTexture(int width, int height)
{
    std::vector<float> data(width * height, 0.0f);
    const float halfAngle = 32.0f * Pi / 180.0f;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const float u = normalizedCoord(x, width);
            const float v = normalizedCoord(y, height);
            const float dx = u - 0.5f;
            const float dy = v - 0.06f;

            if (dy <= 0.0f)
            {
                continue;
            }

            const float angle = std::abs(std::atan2(dx, dy));
            const float radius = std::sqrt(dx * dx + dy * dy);
            const bool insideSector = angle <= halfAngle && radius <= 0.92f;
            data[y * width + x] = insideSector ? 1.0f : 0.0f;
        }
    }

    return data;
}
}

App::App()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(WindowWidth, WindowHeight, "Ultrasound Prototype", nullptr, nullptr);
    if (!window_)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }

    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD.");
    }

    glViewport(0, 0, WindowWidth, WindowHeight);
    createRenderTargets();
    createProceduralInputTextures();
}

App::~App()
{
    combineTarget_.reset();
    echoMaskTexture_.reset();
    lungsTexture_.reset();
    metalTexture_.reset();
    noiseTexture_.reset();
    densityTexture_.reset();

    if (window_)
    {
        glfwDestroyWindow(window_);
    }

    glfwTerminate();
}

void App::createRenderTargets()
{
    combineTarget_ = std::make_unique<Framebuffer>(WindowWidth, WindowHeight, FramebufferFormat::RGBA8);
}

void App::createProceduralInputTextures()
{
    const std::vector<float> densityData = createDensityTexture(ProceduralTextureSize, ProceduralTextureSize);
    const std::vector<float> noiseData = createNoiseTexture(ProceduralTextureSize, ProceduralTextureSize);
    const std::vector<float> metalData = createMetalTexture(ProceduralTextureSize, ProceduralTextureSize);
    const std::vector<float> lungsData = createLungsTexture(ProceduralTextureSize, ProceduralTextureSize);
    const std::vector<float> echoMaskData = createEchoMaskTexture(ProceduralTextureSize, ProceduralTextureSize);

    densityTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, densityData.data());
    noiseTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, noiseData.data());
    metalTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, metalData.data());
    lungsTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, lungsData.data());
    echoMaskTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, echoMaskData.data());
}

void App::presentTexture(unsigned int textureId, Shader& presentShader, FullscreenQuad& quad) const
{
    int framebufferWidth = WindowWidth;
    int framebufferHeight = WindowHeight;
    glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);

    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(0.02f, 0.03f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    presentShader.use();
    presentShader.setInt("buf0", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    quad.draw();

    glBindTexture(GL_TEXTURE_2D, 0);
}

int App::run()
{
    const PipelineParser parser;
    const std::vector<PipelineStage> stages = parser.load(pipelinePath("ultrasound.pipeline.xml"));
    printPipelineCommands(stages);
    const PipelineStage& combineStage = findPipelineStage(stages, "Combine");

    Shader combineShader(shaderPath("fullscreen.vert"), shaderPath("combine.frag"));
    Shader presentShader(shaderPath("fullscreen.vert"), shaderPath("present.frag"));
    FullscreenQuad quad;
    PipelineExecutor executor;
    PipelineResources resources;
    resources.renderTargets["COMBINE"] = combineTarget_.get();
    resources.textures["DENSITY"] = densityTexture_.get();
    resources.textures["NOISE"] = noiseTexture_.get();
    resources.textures["METAL"] = metalTexture_.get();
    resources.textures["LUNGS_IN"] = lungsTexture_.get();
    resources.textures["ECHOMASK"] = echoMaskTexture_.get();
    resources.shaders["UBER"] = &combineShader;
    resources.quad = &quad;

    while (!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();

        executor.executeStage(combineStage, resources);
        Framebuffer::unbind();
        presentTexture(combineTarget_->textureId(), presentShader, quad);

        glfwSwapBuffers(window_);
    }

    return 0;
}
