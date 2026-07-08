#pragma once

struct GLFWwindow;

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
};
