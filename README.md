# OpenGLEngine

Small CMake/OpenGL prototype for an ultrasound rendering experiment inspired by an old Horde3D-style pipeline.

This first pass only opens a 1024x1024 GLFW window, initializes OpenGL through GLAD, compiles a fullscreen quad shader, and draws a visible fullscreen quad. The ultrasound rendering effect is intentionally not implemented yet.

## Layout

```text
assets/
  materials/
  pipelines/
  shaders/
external/
src/
```

## Requirements

- CMake 3.20+
- A C++17 compiler
- GLFW
- GLAD

The CMake project expects `glfw3` and `glad` CMake config packages to be available. With vcpkg, for example:

```powershell
vcpkg install glfw3 glad
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

Run the resulting `OpenGLEngine` executable from the build output directory.
