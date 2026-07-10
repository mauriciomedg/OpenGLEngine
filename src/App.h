#pragma once

#include <memory>

struct GLFWwindow;
class RenderEngine;

class App
{
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    int run();

private:
    GLFWwindow* window_ = nullptr;
    std::unique_ptr<RenderEngine> renderEngine_;
};
