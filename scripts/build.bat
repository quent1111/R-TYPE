@echo off
setlocal enabledelayedexpansion

set "GREEN=[92m"
set "BLUE=[94m"
set "YELLOW=[93m"
set "RED=[91m"
set "NC=[0m"

echo %BLUE%========================================%NC%
echo %BLUE%R-Type Project Build Script (Windows)%NC%
echo %BLUE%========================================%NC%
echo.

set "BUILD_DIR=build"
set "BUILD_TYPE=%~1"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

where conan >nul 2>&1
if %errorlevel% neq 0 (
    echo %RED%Error: Conan is not installed!%NC%
    echo %YELLOW%Install it with: pip install conan%NC%
    exit /b 1
)

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo %RED%Error: CMake is not installed!%NC%
    echo %YELLOW%Install it from: https://cmake.org/download/%NC%
    exit /b 1
)

for /f "tokens=*" %%i in ('conan --version') do set "CONAN_VER=%%i"
for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set "CMAKE_VER=%%i"

echo %GREEN%✓ Conan found: %CONAN_VER%%NC%
echo %GREEN%✓ CMake found: %CMAKE_VER%%NC%
echo.

:: Check if Conan profile exists
if not exist "%USERPROFILE%\.conan2\profiles\default" (
    echo %YELLOW%⚙ Conan profile not found, creating one...%NC%
    conan profile detect --force
    if %errorlevel% neq 0 (
        echo %RED%Error: Failed to create Conan profile!%NC%
        exit /b 1
    )
    echo %GREEN%✓ Conan profile created%NC%
    echo.
)

echo %BLUE%[1/4] Installing dependencies with Conan...%NC%
conan install . --output-folder=%BUILD_DIR% --build=missing -s build_type=%BUILD_TYPE%
if %errorlevel% neq 0 (
    echo %RED%Error: Conan installation failed!%NC%
    exit /b 1
)
echo %GREEN%✓ Dependencies installed%NC%
echo.

echo %BLUE%[2/4] Configuring CMake...%NC%
cmake -B %BUILD_DIR% -S . -DCMAKE_TOOLCHAIN_FILE=%BUILD_DIR%\build\%BUILD_TYPE%\generators\conan_toolchain.cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 (
    echo %RED%Error: CMake configuration failed!%NC%
    exit /b 1
)
echo %GREEN%✓ CMake configured%NC%
echo.

echo %BLUE%[3/4] Building the project...%NC%
cmake --build %BUILD_DIR% --config %BUILD_TYPE% -j %NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo %RED%Error: Build failed!%NC%
    exit /b 1
)
echo %GREEN%✓ Build complete%NC%
echo.

echo %BLUE%[4/4] Build Summary%NC%
echo %GREEN%========================================%NC%
echo %GREEN%✓ Build completed successfully!%NC%
echo %GREEN%========================================%NC%
echo.
echo %YELLOW%Executables location:%NC%
echo   Server: %GREEN%%BUILD_DIR%\bin\%BUILD_TYPE%\r-type_server.exe%NC%
echo   Client: %GREEN%%BUILD_DIR%\bin\%BUILD_TYPE%\r-type_client.exe%NC%
echo.
echo %YELLOW%To run the server:%NC%
echo   cd %BUILD_DIR%\bin\%BUILD_TYPE% ^&^& r-type_server.exe
echo.
echo %YELLOW%To run the client:%NC%
echo   cd %BUILD_DIR%\bin\%BUILD_TYPE% ^&^& r-type_client.exe
echo.
echo %YELLOW%To run tests:%NC%
echo   cd %BUILD_DIR% ^&^& ctest --output-on-failure -C %BUILD_TYPE%
echo.
echo %BLUE%========================================%NC%

cd ..
