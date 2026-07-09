#pragma once

#include <memory>

struct GLFWwindow;
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
    void createProceduralInputTextures();
    void bindProceduralInputTextures() const;

    GLFWwindow* window_ = nullptr;
    std::unique_ptr<Texture2D> densityTexture_;
    std::unique_ptr<Texture2D> noiseTexture_;
    std::unique_ptr<Texture2D> metalTexture_;
    std::unique_ptr<Texture2D> lungsTexture_;
    std::unique_ptr<Texture2D> echoMaskTexture_;
};
