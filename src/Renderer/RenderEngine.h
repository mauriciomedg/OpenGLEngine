#pragma once

#include "../Pipeline/PipelineExecutor.h"
#include "../Pipeline/PipelineParser.h"
#include "../Material/MaterialManager.h"

#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

class Camera;
class Framebuffer;
class FullscreenQuad;
class Mesh;
class Shader;
class Texture2D;

class RenderEngine
{
public:
    RenderEngine(int initialWindowWidth, int initialWindowHeight);
    ~RenderEngine();

    RenderEngine(const RenderEngine&) = delete;
    RenderEngine& operator=(const RenderEngine&) = delete;

    void resize(int framebufferWidth, int framebufferHeight);
    void renderFrame(float timeSeconds);

private:
    void loadPipelines();
    void createShaders();
    void createRenderTargets();
    void createProceduralTextures();
    void createScene();
    void registerResources();
    void registerGeometryCallbacks();
    void registerShaderSetups();

    void updateFrameState(float timeSeconds);
    void configureLeftViewport();
    void presentUltrasound();

    int framebufferWidth_ = 0;
    int framebufferHeight_ = 0;

    std::unique_ptr<Camera> camera_;

    std::unique_ptr<Framebuffer> shadowTarget_;
    std::unique_ptr<Framebuffer> densityTarget_;
    std::unique_ptr<Framebuffer> combineTarget_;

    std::unique_ptr<Texture2D> noiseTexture_;
    std::unique_ptr<Texture2D> metalTexture_;
    std::unique_ptr<Texture2D> lungsTexture_;
    std::unique_ptr<Texture2D> echoMaskTexture_;

    std::unique_ptr<Shader> presentShader_;
    std::unique_ptr<Shader> meshShader_;
    std::unique_ptr<Shader> shadowShader_;

    std::unique_ptr<FullscreenQuad> fullscreenQuad_;
    std::unique_ptr<Mesh> cube_;

    PipelineParser pipelineParser_;
    PipelineExecutor pipelineExecutor_;
    MaterialManager materialManager_;
    RendererResources resources_;

    std::vector<PipelineStage> forwardStages_;
    std::vector<PipelineStage> sliceStages_;
    std::vector<PipelineStage> ultrasoundStages_;
    const PipelineStage* shadowsStage_ = nullptr;
    const PipelineStage* geometryStage_ = nullptr;
    const PipelineStage* sliceStage_ = nullptr;
    const PipelineStage* combineStage_ = nullptr;

    glm::mat4 cubeModel_{1.0f};
    glm::mat4 inverseCubeModel_{1.0f};
    glm::mat4 lightViewProjection_{1.0f};
};
