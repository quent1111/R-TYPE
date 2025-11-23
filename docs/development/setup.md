# Development Setup Guide

This guide will help you set up your development environment for the R-Type project.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Package Manager Setup (Conan)](#package-manager-setup-conan)
- [Building the Project](#building-the-project)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required Software

#### All Platforms
- **CMake** 3.20 or higher
  - Download: https://cmake.org/download/
  - Verify: `cmake --version`

- **Python** 3.7 or higher (for Conan)
  - Download: https://www.python.org/downloads/
  - Verify: `python --version` or `python3 --version`

- **Git**
  - Download: https://git-scm.com/downloads
  - Verify: `git --version`

#### Linux
- **GCC** 9+ or **Clang** 10+
  ```bash
  sudo apt update
  sudo apt install build-essential cmake python3 python3-pip git
  ```

#### Windows
- **Visual Studio 2019** or newer (with C++ Desktop Development workload)
  - Download: https://visualstudio.microsoft.com/downloads/
  - Or **MinGW-w64** for GCC on Windows

---

## Package Manager Setup (Conan)

### Why Conan?

We chose **Conan** as our package manager because:
- ✅ Cross-platform support (Linux, Windows, macOS)
- ✅ Binary caching for faster CI/CD
- ✅ Version locking for reproducible builds
- ✅ Industry standard for C++ projects

### 1. Install Conan

```bash
pip install conan
```

Or on some systems:
```bash
pip3 install conan
```

Verify installation:
```bash
conan --version
```

You should see something like `Conan version 2.x.x`.

### 2. Configure Conan Profile

Conan needs to detect your compiler and platform settings.

**Auto-detect (recommended):**
```bash
conan profile detect --force
```

**Verify your profile:**
```bash
conan profile show default
```

You should see output like:
```
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=11
build_type=Release
```

### 3. Install Project Dependencies

From the project root directory:

```bash
conan install . --output-folder=build --build=missing
```

**What this does:**
- Reads `conanfile.txt` in the project root
- Downloads and installs SFML, Asio, and GTest
- Generates CMake configuration files in `build/` folder
- Builds any missing binary packages

**First-time setup?** This might take 5-10 minutes as Conan builds dependencies.

---

## Building the Project

### Linux / macOS

#### Option 1: Using the build script (Recommended)

```bash
./scripts/build.sh
```

This script automatically:
1. Installs dependencies with Conan
2. Configures CMake
3. Builds the project
4. Shows you how to run the executables

#### Option 2: Manual build

```bash
# 1. Install dependencies
conan install . --output-folder=build --build=missing

# 2. Configure CMake
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build . --config Release

# 4. Run
./bin/r-type_server
./bin/r-type_client
```

### Windows (Visual Studio)

#### Using Command Prompt or PowerShell:

```powershell
# 1. Install dependencies
conan install . --output-folder=build --build=missing

# 2. Configure CMake
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

# 3. Build
cmake --build . --config Release

# 4. Run
.\bin\Release\r-type_server.exe
.\bin\Release\r-type_client.exe
```

### Clean Build

To start fresh:

```bash
rm -rf build/
conan install . --output-folder=build --build=missing
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build .
```

---

## Troubleshooting

### Common Issues

#### ❌ `conan: command not found`

**Solution:** Conan is not in your PATH. Try:
```bash
pip3 install --user conan
export PATH="$HOME/.local/bin:$PATH"  # Linux/macOS
```

Or install globally (may require sudo):
```bash
sudo pip3 install conan
```

---

#### ❌ `CMake Error: could not find CMAKE_TOOLCHAIN_FILE`

**Problem:** CMake can't find the Conan-generated toolchain.

**Solution:** Make sure you ran `conan install` first:
```bash
conan install . --output-folder=build --build=missing
```

Then use the correct path:
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
```

---

#### ❌ `ERROR: Missing prebuilt package`

**Problem:** Conan can't find a pre-built binary for your platform.

**Solution:** Build from source with `--build=missing`:
```bash
conan install . --output-folder=build --build=missing
```

---

#### ❌ Compiler not detected or wrong version

**Problem:** Conan detected the wrong compiler.

**Solution:** Manually edit your Conan profile:
```bash
conan profile show default
conan profile path default  # Shows profile location
```

Edit the profile and set the correct compiler, then:
```bash
conan install . --output-folder=build --build=missing
```

---

#### ❌ SFML/OpenGL errors on Linux

**Problem:** Missing system graphics libraries.

**Solution:** Install development packages:

**Ubuntu/Debian:**
```bash
sudo apt install libx11-dev libxrandr-dev libxcursor-dev libxi-dev \
                 libudev-dev libgl1-mesa-dev libopenal-dev libflac-dev \
                 libvorbis-dev
```

**Fedora:**
```bash
sudo dnf install libX11-devel libXrandr-devel libXcursor-devel libXi-devel \
                 systemd-devel mesa-libGL-devel openal-soft-devel \
                 flac-devel libvorbis-devel
```

---

#### ❌ Windows: MSVC not found

**Problem:** Visual Studio not detected.

**Solution:**
1. Install **Visual Studio 2019+** with "Desktop development with C++" workload
2. Run from "Developer Command Prompt for VS"
3. Or set environment manually:
   ```powershell
   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
   ```

---

#### ❌ Build fails with linking errors

**Problem:** Mixed Debug/Release libraries.

**Solution:** Make sure to use the same build type everywhere:
```bash
# For Release builds
conan install . --output-folder=build --build=missing -s build_type=Release
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# For Debug builds
conan install . --output-folder=build --build=missing -s build_type=Debug
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
```

---

## CI/CD Notes

### GitHub Actions Example

The project uses dependency caching to avoid re-downloading on every commit:

```yaml
- name: Cache Conan packages
  uses: actions/cache@v3
  with:
    path: ~/.conan2
    key: ${{ runner.os }}-conan-${{ hashFiles('**/conanfile.txt') }}
```

This dramatically speeds up CI builds (from ~10 minutes to ~2 minutes).

---

## Additional Resources

- **Conan Documentation:** https://docs.conan.io/
- **CMake Tutorial:** https://cmake.org/cmake/help/latest/guide/tutorial/
- **SFML Documentation:** https://www.sfml-dev.org/documentation/
- **Asio Documentation:** https://think-async.com/Asio/

---

## Getting Help

If you encounter issues not covered here:

1. Check the [GitHub Issues](https://github.com/quent1111/R-TYPE/issues)
2. Ask in the team chat
3. Consult `docs/CONTRIBUTING.md` for development guidelines

---

**Happy Coding! 🚀**
