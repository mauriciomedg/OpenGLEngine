#pragma once

#include <glm/mat4x4.hpp>

#include <memory>

struct GLFWwindow;
class Framebuffer;
class FullscreenQuad;
class Mesh;
class Shader;
class Texture2D;
class Camera;

class App
{
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    int run();

private:
    void createRenderTargets();
    void createProceduralInputTextures();
    void renderCubeViewport(Mesh& cube, Shader& meshShader, const glm::mat4& model) const;
    void renderDensitySlice(Shader& sliceShader, FullscreenQuad& quad, const glm::mat4& inverseModel) const;
    void presentTexture(unsigned int textureId, Shader& presentShader, FullscreenQuad& quad, int x, int y, int width, int height) const;

    GLFWwindow* window_ = nullptr;
    std::unique_ptr<Camera> camera_;

    std::unique_ptr<Framebuffer> densityTarget_;
    std::unique_ptr<Framebuffer> combineTarget_;
    std::unique_ptr<Texture2D> noiseTexture_;
    std::unique_ptr<Texture2D> metalTexture_;
    std::unique_ptr<Texture2D> lungsTexture_;
    std::unique_ptr<Texture2D> echoMaskTexture_;
};
