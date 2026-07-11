cmake_minimum_required(VERSION 3.20)

project(OpenGLEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)

add_executable(OpenGLEngine
    src/main.cpp
    src/App.h
    src/App.cpp
    src/Shader.h
    src/Shader.cpp
    src/FullscreenQuad.h
    src/FullscreenQuad.cpp
    src/Material/Material.h
    src/Material/Material.cpp
    src/Material/MaterialManager.h
    src/Material/MaterialManager.cpp
    src/Profiling/ScopedGpuDebugGroup.h
    src/Profiling/GpuTimer.h
    src/Profiling/GpuTimer.cpp
    src/Profiling/FrameProfiler.h
    src/Profiling/FrameProfiler.cpp
    src/Pipeline/PipelineExecutor.h
    src/Pipeline/PipelineExecutor.cpp
    src/Pipeline/PipelineParser.h
    src/Pipeline/PipelineParser.cpp
    src/Renderer/Framebuffer.h
    src/Renderer/Framebuffer.cpp
    src/Renderer/Mesh.h
    src/Renderer/Mesh.cpp
    src/Renderer/Camera.h
    src/Renderer/Camera.cpp
    src/Renderer/RenderEngine.h
    src/Renderer/RenderEngine.cpp
    src/Renderer/Texture2D.h
    src/Renderer/Texture2D.cpp
    src/Renderer/TextureFormat.h
    src/Renderer/TextureFormat.cpp
)

target_link_libraries(OpenGLEngine PRIVATE glfw glad::glad glm::glm tinyxml2::tinyxml2)

target_compile_definitions(OpenGLEngine PRIVATE
    ASSET_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

if (MSVC)
    target_compile_options(OpenGLEngine PRIVATE /W4 /permissive-)
else()
    target_compile_options(OpenGLEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET OpenGLEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:OpenGLEngine>/assets
)
