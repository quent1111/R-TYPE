# Installation Guide

This guide will help you install and set up the R-TYPE project on your system.

## Prerequisites

### Required Tools

- **C++ Compiler** with C++20 support
  - GCC 10+ (Linux)
  - Clang 12+ (macOS)
  - MSVC 2019+ or MinGW-w64 GCC 10+ (Windows)
- **CMake** 3.20 or higher
- **Python** 3.7+ with pip (for Conan package manager)
- **Git** for version control

### Dependencies (Auto-Installed)

The following dependencies are **automatically installed** via Conan:

- **SFML 2.6.1** - Graphics, window, and audio
- **Asio 1.30.2** - Networking library
- **GTest 1.14.0** - Unit testing framework
- **LZ4 1.9.4** - Compression library

You don't need to install them manually!

## Platform-Specific Setup

### Linux (Ubuntu/Debian)

```bash
# Update package list
sudo apt update

# Install build essentials
sudo apt install build-essential cmake git python3 python3-pip

# Verify installations
cmake --version    # Should be 3.20+
g++ --version      # Should be GCC 10+
python3 --version  # Should be 3.7+
```

### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required tools
brew install cmake git python

# For ARM64 Macs, also install SFML via Homebrew
brew install sfml

# Verify installations
cmake --version
clang++ --version  # Should be Clang 12+
python3 --version
```

### Windows (MinGW-w64 via MSYS2)

**Option 1: MSYS2 (Recommended)**

1. **Download and install MSYS2** from [msys2.org](https://www.msys2.org/)

2. **Open MSYS2 MinGW 64-bit terminal** and run:
   ```bash
   # Update package database
   pacman -Syu
   
   # Install build tools
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
             mingw-w64-x86_64-make git python python-pip
   
   # Add to PATH (in Windows System Environment Variables):
   # C:\msys64\mingw64\bin
   ```

**Option 2: Visual Studio**

1. **Install Visual Studio 2019 or later**
   - Download from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
   - Select "Desktop development with C++" workload

2. **Install CMake**
   - Download from [cmake.org](https://cmake.org/download/)
   - Add to PATH during installation

3. **Install Python**
   - Download from [python.org](https://www.python.org/downloads/)
   - Check "Add Python to PATH" during installation

## Cloning the Repository

```bash
# Clone the repository
git clone https://github.com/quent1111/R-TYPE.git

# Navigate to the project directory
cd R-TYPE
```

## Quick Build (Recommended)

The **easiest way** to build R-TYPE:

### Linux/macOS

```bash
# Build and run server (auto-installs everything!)
./r-type.sh server

# Or just build without running
./r-type.sh build
```

### Windows

```cmd
REM Build and run server
r-type.bat server

REM Or just build without running
r-type.bat build
```

The script automatically:
- ✅ Installs Conan package manager (if not present)
- ✅ Downloads and builds all dependencies
- ✅ Configures CMake with correct toolchain
- ✅ Builds the project in parallel

## Manual Build (Advanced)

If you prefer manual control:

### Step 1: Install Conan

```bash
pip install conan
conan profile detect --force
```

### Step 2: Install Dependencies

```bash
conan install . --output-folder=build --build=missing -s build_type=Release
```

This downloads and builds SFML, Asio, GTest, and LZ4.

### Step 3: Configure CMake

```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

### Step 4: Build

```bash
# Linux/macOS
cmake --build . -j$(nproc)

# Windows
cmake --build . --config Release
```

### Step 5: Run

```bash
# Server
./bin/r-type_server

# Client (in another terminal)
./bin/r-type_client
```

## Build Options

### Debug Build

For development with debugging symbols:

```bash
./r-type.sh build --debug

# Or manually:
conan install . --output-folder=build --build=missing -s build_type=Debug
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### With Tests

```bash
./r-type.sh test

# This builds with tests enabled and runs them
```

### Clean Rebuild

```bash
./r-type.sh rebuild

# Or manually:
rm -rf build
./r-type.sh build
```

## Verifying Installation

After building, verify that everything works:

```bash
# Run all tests
./r-type.sh test

# Or manually run tests
cd build
ctest --output-on-failure

# Run the server
./r-type.sh server

# In another terminal, run the client
./r-type.sh client
```

**Expected output:**
- Tests: All 456 tests should pass
- Server: Should start and listen on port 4242
- Client: Game window should open and connect to server

## Available Commands

The `r-type.sh` (or `r-type.bat` on Windows) script supports these commands:

| Command | Description |
|---------|-------------|
| `build` | Build the entire project |
| `client` | Build and run the client |
| `server` | Build and run the server |
| `admin` | Build and run the admin client |
| `test` | Run all tests with CTest |
| `coverage` | Generate code coverage report |
| `valgrind` | Run with Valgrind memory check |
| `clean` | Clean build directory |
| `install` | Install dependencies only |
| `rebuild` | Clean and rebuild from scratch |

**Options:**
- `--debug` - Build in Debug mode
- `--release` - Build in Release mode (default)
- `--verbose` - Show verbose output
- `--clean` - Clean before building
- `-j N` - Use N parallel jobs

**Examples:**
```bash
# Debug build with 8 jobs
./r-type.sh build --debug -j 8

# Clean rebuild
./r-type.sh rebuild

# Verbose test output
./r-type.sh test --verbose
```

## Next Steps

- 📖 [Quick Start Guide](quickstart.md) - Learn how to play R-TYPE
- 🏗️ [Building Guide](building.md) - Advanced build configurations
- � [Testing Guide](../developer-guide/testing.md) - Run and write tests
- �🤝 [Contributing](../developer-guide/contributing.md) - Start contributing

## Troubleshooting

### Conan Not Found

After installing Conan with pip, it may not be in PATH:

```bash
# Add to PATH (Linux/macOS)
export PATH="$HOME/.local/bin:$PATH"

# Or use full path
python3 -m pip install conan
```

### CMake Version Too Old

```bash
# Ubuntu/Debian - Install from Kitware APT repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install cmake

# macOS
brew upgrade cmake

# Windows - Download latest from cmake.org
```

### Compiler Not C++20 Compatible

```bash
# Linux - Install newer GCC
sudo apt install gcc-10 g++-10
export CC=gcc-10
export CXX=g++-10

# macOS - Update Xcode Command Line Tools
xcode-select --install

# Windows - Use Visual Studio 2019+ or update MinGW
```

### SFML Dependencies Missing (Linux)

Some systems may need additional SFML dependencies:

```bash
sudo apt install libx11-dev libxrandr-dev libxcursor-dev \
                 libxi-dev libudev-dev libgl1-mesa-dev \
                 libopenal-dev libvorbis-dev libflac-dev
```

### Build Fails on Windows

**Issue:** `cannot find -lsfml-graphics`

**Solution:** Use MinGW-w64 from MSYS2 or Visual Studio, not standard MinGW:
```bash
# In MSYS2 MinGW 64-bit terminal
pacman -S mingw-w64-x86_64-toolchain
```

### Port 4242 Already in Use

Change the server port:

```bash
./r-type.sh server -- --port 5000

# Or edit settings.ini
[Server]
port = 5000
```

### Tests Fail

```bash
# Make sure you're building in Release mode for tests
./r-type.sh build --release
./r-type.sh test

# Run specific test for debugging
cd build
./bin/test_registry --gtest_filter="EntityTest.*"
```

## Need Help?

- 📝 Check the [FAQ](../faq.md)
- 🐛 [Report an issue](https://github.com/quent1111/R-TYPE/issues)
- � [Full Documentation](https://quent1111.github.io/r-type)
- 🏗️ [Architecture Overview](../architecture/overview.md)
- 🌐 [Network Documentation](../architecture/network.md)
