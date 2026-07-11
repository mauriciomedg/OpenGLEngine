#include "RenderEngine.h"

#include "../FullscreenQuad.h"
#include "../Profiling/FrameProfiler.h"
#include "../Profiling/ScopedGpuDebugGroup.h"
#include "../Shader.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Mesh.h"
#include "Texture2D.h"

#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include <algorithm>
#include <chrono>
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

float normalizedCoord(int value, int size)
{
    return (static_cast<float>(value) + 0.5f) / static_cast<float>(size);
}

glm::mat4 createCubeModelMatrix(float time)
{
    glm::mat4 model(1.0f);
    model = glm::rotate(model, time * 0.85f, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, time * 0.45f, glm::vec3(1.0f, 0.0f, 0.0f));
    return model;
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

void labelOpenGlObject(GLenum identifier, GLuint object, const char* name)
{
    if (object != 0 && glObjectLabel != nullptr)
    {
        glObjectLabel(identifier, object, -1, name);
    }
}

class ScopedProfileTimer
{
public:
    ScopedProfileTimer(FrameProfiler& profiler, ProfileStage stage)
        : profiler_(profiler)
        , stage_(stage)
    {
        profiler_.begin(stage_);
    }

    ~ScopedProfileTimer()
    {
        profiler_.end(stage_);
    }

    ScopedProfileTimer(const ScopedProfileTimer&) = delete;
    ScopedProfileTimer& operator=(const ScopedProfileTimer&) = delete;

private:
    FrameProfiler& profiler_;
    ProfileStage stage_;
};
}

RenderEngine::RenderEngine(int initialWindowWidth, int initialWindowHeight)
    : framebufferWidth_(initialWindowWidth)
    , framebufferHeight_(initialWindowHeight)
{
    loadPipelines();
    createShaders();
    createRenderTargets();
    labelRenderTargets();
    createProceduralTextures();
    createScene();
    frameProfiler_ = std::make_unique<FrameProfiler>();
    registerResources();
    registerGeometryCallbacks();
    registerShaderSetups();
}

RenderEngine::~RenderEngine() = default;

void RenderEngine::resize(int framebufferWidth, int framebufferHeight)
{
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        return;
    }

    framebufferWidth_ = framebufferWidth;
    framebufferHeight_ = framebufferHeight;
}

void RenderEngine::loadPipelines()
{
    forwardStages_ = pipelineParser_.load(pipelinePath("forward.pipeline.xml"));
    sliceStages_ = pipelineParser_.load(pipelinePath("slice.pipeline.xml"));
    ultrasoundStages_ = pipelineParser_.load(pipelinePath("ultrasound.pipeline.xml"));

    shadowsStage_ = &findPipelineStage(forwardStages_, "Shadows");
    geometryStage_ = &findPipelineStage(forwardStages_, "Geometry");
    sliceStage_ = &findPipelineStage(sliceStages_, "Slice");
    combineStage_ = &findPipelineStage(ultrasoundStages_, "Combine");
}

void RenderEngine::createShaders()
{
    presentShader_ = std::make_unique<Shader>(shaderPath("fullscreen.vert"), shaderPath("present.frag"));
    depthDebugShader_ = std::make_unique<Shader>(shaderPath("fullscreen.vert"), shaderPath("depth_debug.frag"));
    meshShader_ = std::make_unique<Shader>(shaderPath("mesh.vert"), shaderPath("mesh.frag"));
    shadowShader_ = std::make_unique<Shader>(shaderPath("shadow_depth.vert"), shaderPath("shadow_depth.frag"));
}

void RenderEngine::createRenderTargets()
{
    shadowTarget_ = std::make_unique<Framebuffer>(FramebufferDesc{
        1024,
        1024,
        false,
        true,
        FramebufferFormat::Depth32F});
    densityTarget_ = std::make_unique<Framebuffer>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F);
    combineTarget_ = std::make_unique<Framebuffer>(WindowWidth, WindowHeight, FramebufferFormat::RGBA8);
}

void RenderEngine::labelRenderTargets()
{
    labelOpenGlObject(GL_FRAMEBUFFER, shadowTarget_->fboId(), "Framebuffer/Shadow");
    labelOpenGlObject(GL_TEXTURE, shadowTarget_->depthTextureId(), "Texture/ShadowDepth");
    labelOpenGlObject(GL_FRAMEBUFFER, densityTarget_->fboId(), "Framebuffer/Density");
    labelOpenGlObject(GL_TEXTURE, densityTarget_->textureId(), "Texture/Density");
    labelOpenGlObject(GL_FRAMEBUFFER, combineTarget_->fboId(), "Framebuffer/Combine");
    labelOpenGlObject(GL_TEXTURE, combineTarget_->textureId(), "Texture/Combine");
}

void RenderEngine::createProceduralTextures()
{
    const std::vector<float> noiseData = createNoiseTexture(ProceduralTextureSize, ProceduralTextureSize);
    const std::vector<float> metalData = createMetalTexture(ProceduralTextureSize, ProceduralTextureSize);
    const std::vector<float> lungsData = createLungsTexture(ProceduralTextureSize, ProceduralTextureSize);
    const std::vector<float> echoMaskData = createEchoMaskTexture(ProceduralTextureSize, ProceduralTextureSize);

    noiseTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, noiseData.data());
    metalTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, metalData.data());
    lungsTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, lungsData.data());
    echoMaskTexture_ = std::make_unique<Texture2D>(ProceduralTextureSize, ProceduralTextureSize, FramebufferFormat::R32F, echoMaskData.data());
}

void RenderEngine::createScene()
{
    camera_ = std::make_unique<Camera>();
    fullscreenQuad_ = std::make_unique<FullscreenQuad>();
    cube_ = std::make_unique<Mesh>(Mesh::createCube());
    groundPlane_ = std::make_unique<Mesh>(Mesh::createPlane());
    lightViewProjection_ =
        glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 12.0f) *
        glm::lookAt(glm::vec3(2.0f, 3.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void RenderEngine::registerResources()
{
    resources_.renderTargets["SHADOWBUFS"] = shadowTarget_.get();
    resources_.renderTargets["DENSITY"] = densityTarget_.get();
    resources_.renderTargets["COMBINE"] = combineTarget_.get();

    resources_.textures["NOISE"] = noiseTexture_.get();
    resources_.textures["METAL"] = metalTexture_.get();
    resources_.textures["LUNGS_IN"] = lungsTexture_.get();
    resources_.textures["ECHOMASK"] = echoMaskTexture_.get();

    resources_.materialManager = &materialManager_;
    resources_.assetRoot = ASSET_ROOT;
    resources_.shaders["SHADOWS"] = shadowShader_.get();
    resources_.shaders["LIGHTING"] = meshShader_.get();

    resources_.quad = fullscreenQuad_.get();
}

void RenderEngine::registerGeometryCallbacks()
{
    resources_.geometryDraws["SHADOWS"] =
        [this]()
        {
            glEnable(GL_DEPTH_TEST);
            shadowShader_->use();

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2.0f, 4.0f);

            shadowShader_->setMat4("uModel", cubeModel_);
            shadowShader_->setMat4("uLightViewProjection", lightViewProjection_);
            cube_->draw();

            shadowShader_->setMat4("uModel", groundModel_);
            groundPlane_->draw();

            glDisable(GL_POLYGON_OFFSET_FILL);

            static bool logged = false;
            if (!logged)
            {
                std::cout << "[Shadow] Rendered depth map "
                          << shadowTarget_->width() << "x" << shadowTarget_->height() << '\n';
                logged = true;
            }
        };

    resources_.geometryDraws["LIGHTING"] =
        [this]()
        {
            glEnable(GL_DEPTH_TEST);
            meshShader_->use();
            meshShader_->setMat4("uLightViewProjection", lightViewProjection_);
            meshShader_->setVec3("uLightDirection", glm::normalize(glm::vec3(-2.0f, -3.0f, -2.0f)));
            meshShader_->setVec3("uLightColor", glm::vec3(1.0f));
            meshShader_->setVec3("uAmbientColor", glm::vec3(0.18f));
            meshShader_->setBool("uEnablePcf", true);

            meshShader_->setMat4("uModel", cubeModel_);
            meshShader_->setMat4("uView", camera_->viewMatrix());
            meshShader_->setMat4("uProjection", camera_->projectionMatrix());
            cube_->draw();

            meshShader_->setMat4("uModel", groundModel_);
            groundPlane_->draw();

            static bool logged = false;
            if (!logged)
            {
                std::cout << "[Shadow] PCF enabled\n";
                logged = true;
            }
        };
}

void RenderEngine::registerShaderSetups()
{
    resources_.shaderSetups["SLICE"] =
        [this](Shader& shader)
        {
            shader.setMat4("uInverseModel", inverseCubeModel_);
            shader.setFloat("uSliceZ", 0.0f);
        };
}

void RenderEngine::updateFrameState(float timeSeconds)
{
    cubeModel_ = createCubeModelMatrix(timeSeconds);
    inverseCubeModel_ = glm::inverse(cubeModel_);

    const int leftWidth = framebufferWidth_ / 2;
    if (leftWidth > 0 && framebufferHeight_ > 0)
    {
        camera_->setAspectRatio(static_cast<float>(leftWidth) / static_cast<float>(framebufferHeight_));
    }
}

void RenderEngine::configureLeftViewport()
{
    const int leftWidth = framebufferWidth_ / 2;

    glEnable(GL_SCISSOR_TEST);
    glViewport(0, 0, leftWidth, framebufferHeight_);
    glScissor(0, 0, leftWidth, framebufferHeight_);
}

void RenderEngine::presentUltrasound()
{
    const int leftWidth = framebufferWidth_ / 2;
    const int rightWidth = framebufferWidth_ - leftWidth;

    Framebuffer::unbind();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glViewport(leftWidth, 0, rightWidth, framebufferHeight_);
    glScissor(leftWidth, 0, rightWidth, framebufferHeight_);
    glClearColor(0.02f, 0.03f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    presentShader_->use();
    presentShader_->setInt("buf0", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, combineTarget_->textureId());

    fullscreenQuad_->draw();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_SCISSOR_TEST);
}

void RenderEngine::presentShadowDebug()
{
    const int leftWidth = framebufferWidth_ / 2;
    const int rightWidth = framebufferWidth_ - leftWidth;

    Framebuffer::unbind();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glViewport(leftWidth, 0, rightWidth, framebufferHeight_);
    glScissor(leftWidth, 0, rightWidth, framebufferHeight_);
    glClearColor(0.02f, 0.03f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    depthDebugShader_->use();
    depthDebugShader_->setInt("buf0", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowTarget_->depthTextureId());

    fullscreenQuad_->draw();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_SCISSOR_TEST);
}

void RenderEngine::renderFrame(float timeSeconds)
{
    const auto cpuStart = std::chrono::steady_clock::now();

    updateFrameState(timeSeconds);

    glDisable(GL_SCISSOR_TEST);
    {
        ScopedGpuDebugGroup marker("Frame/Shadows");
        ScopedProfileTimer timer(*frameProfiler_, ProfileStage::Shadow);
        pipelineExecutor_.executeStage(*shadowsStage_, resources_);
    }

    configureLeftViewport();
    {
        ScopedGpuDebugGroup marker("Frame/Geometry");
        ScopedProfileTimer timer(*frameProfiler_, ProfileStage::Geometry);
        pipelineExecutor_.executeStage(*geometryStage_, resources_);
    }

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);

    {
        ScopedGpuDebugGroup marker("Frame/Slice");
        ScopedProfileTimer timer(*frameProfiler_, ProfileStage::Slice);
        pipelineExecutor_.executeStage(*sliceStage_, resources_);
    }

    {
        ScopedGpuDebugGroup marker("Frame/Combine");
        ScopedProfileTimer timer(*frameProfiler_, ProfileStage::Combine);
        pipelineExecutor_.executeStage(*combineStage_, resources_);
    }

    Framebuffer::unbind();
    {
        ScopedGpuDebugGroup marker("Frame/Present");
        ScopedProfileTimer timer(*frameProfiler_, ProfileStage::Present);

        if (showShadowMap_)
        {
            presentShadowDebug();
        }
        else
        {
            presentUltrasound();
        }
    }

    const auto cpuEnd = std::chrono::steady_clock::now();
    const double cpuFrameMs = std::chrono::duration<double, std::milli>(cpuEnd - cpuStart).count();
    frameProfiler_->endFrame(cpuFrameMs);
}
