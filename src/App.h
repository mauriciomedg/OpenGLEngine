#pragma once

#include <memory>

struct GLFWwindow;
class Framebuffer;
class FullscreenQuad;
class Shader;
class Texture2D;

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
    void presentTexture(unsigned int textureId, Shader& presentShader, FullscreenQuad& quad) const;

    GLFWwindow* window_ = nullptr;
    std::unique_ptr<Framebuffer> combineTarget_;
    std::unique_ptr<Texture2D> densityTexture_;
    std::unique_ptr<Texture2D> noiseTexture_;
    std::unique_ptr<Texture2D> metalTexture_;
    std::unique_ptr<Texture2D> lungsTexture_;
    std::unique_ptr<Texture2D> echoMaskTexture_;
};
