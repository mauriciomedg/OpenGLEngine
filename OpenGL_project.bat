@echo off
setlocal

cd /d "%~dp0"

set CONFIG=Debug
set VCPKG_DIR=C:\vcpkg
set TOOLCHAIN=%VCPKG_DIR%\scripts\buildsystems\vcpkg.cmake

echo Checking vcpkg...

if not exist "%VCPKG_DIR%" (
    echo Installing vcpkg in %VCPKG_DIR%...
    git clone https://github.com/microsoft/vcpkg "%VCPKG_DIR%"
    if errorlevel 1 goto error

    call "%VCPKG_DIR%\bootstrap-vcpkg.bat"
    if errorlevel 1 goto error
)

if not exist "%TOOLCHAIN%" (
    echo Bootstrapping vcpkg...
    call "%VCPKG_DIR%\bootstrap-vcpkg.bat"
    if errorlevel 1 goto error
)

echo Installing OpenGL dependencies...
"%VCPKG_DIR%\vcpkg.exe" install glfw3:x64-windows glad:x64-windows tinyxml2:x64-windows
if errorlevel 1 goto error

if exist build rmdir /s /q build

echo Configuring CMake...
cmake -S . -B build ^
  -G "Visual Studio 17 2022" ^
  -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN%"

if errorlevel 1 goto error

echo Building...
cmake --build build --config %CONFIG%
if errorlevel 1 goto error

echo.
echo Running OpenGLEngine...
echo.

if exist "build\%CONFIG%\OpenGLEngine.exe" (
    "build\%CONFIG%\OpenGLEngine.exe"
) else (
    echo Executable not found: build\%CONFIG%\OpenGLEngine.exe
)

pause
exit /b 0

:error
echo.
echo Build failed.
pause
exit /b 1
