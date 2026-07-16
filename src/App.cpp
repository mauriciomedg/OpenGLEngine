#include "App.h"

#include "Renderer/RenderEngine.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace
{
constexpr int WindowWidth = 1024;
constexpr int WindowHeight = 1024;

void framebufferSizeCallback(GLFWwindow*, int, int)
{
}
}

App::App()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

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

    int framebufferWidth = WindowWidth;
    int framebufferHeight = WindowHeight;
    glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
    renderEngine_ = std::make_unique<RenderEngine>(framebufferWidth, framebufferHeight);
}

App::~App()
{
    renderEngine_.reset();

    if (window_)
    {
        glfwDestroyWindow(window_);
    }

    glfwTerminate();
}

int App::run()
{
    while (!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);

        renderEngine_->resize(framebufferWidth, framebufferHeight);
        renderEngine_->renderFrame(static_cast<float>(glfwGetTime()));

        glfwSwapBuffers(window_);
    }

    return 0;
}
