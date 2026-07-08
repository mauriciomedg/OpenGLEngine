#include "App.h"

#include "FullscreenQuad.h"
#include "Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>

namespace
{
constexpr int WindowWidth = 1024;
constexpr int WindowHeight = 1024;

std::string shaderPath(const char* fileName)
{
    return std::string(ASSET_ROOT) + "/shaders/" + fileName;
}

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
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
}

App::~App()
{
    if (window_)
    {
        glfwDestroyWindow(window_);
    }

    glfwTerminate();
}

int App::run()
{
    Shader shader(shaderPath("fullscreen.vert"), shaderPath("combine.frag"));
    FullscreenQuad quad;

    while (!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();

        glClearColor(0.02f, 0.03f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        quad.draw();

        glfwSwapBuffers(window_);
    }

    return 0;
}
