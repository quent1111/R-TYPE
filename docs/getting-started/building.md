# Building R-TYPE

This guide covers building R-TYPE from source.

## Quick Start (Recommended)

The **easiest way** to build and run R-TYPE is using the unified script:

=== "Linux/macOS"

    ```bash
    # Build and run server
    ./r-type.sh server
    
    # Build and run client
    ./r-type.sh client
    
    # Just build everything
    ./r-type.sh build
    
    # Run tests
    ./r-type.sh test
    ```

=== "Windows"

    ```cmd
    REM Build and run server
    r-type.bat server
    
    REM Build and run client
    r-type.bat client
    
    REM Just build everything
    r-type.bat build
    
    REM Run tests
    r-type.bat test
    ```

!!! success "Automatic Setup"
    The `r-type.sh` and `r-type.bat` scripts automatically:
    
    - ✅ Install Conan package manager
    - ✅ Download and build dependencies (SFML, Asio, GTest)
    - ✅ Configure CMake with the correct toolchain
    - ✅ Build the project in parallel
    - ✅ Run the server or client

---

## All Available Commands

| Command | Description |
|---------|-------------|
| `build` | Build the entire project |
| `client` | Build and run the client |
| `server` | Build and run the server |
| `admin` | Build and run the admin client |
| `test` | Run all tests (456 tests across 7 categories) |
| `coverage` | Generate code coverage report (Linux/macOS) |
| `valgrind` | Run server with Valgrind memory check (Linux) |
| `clean` | Clean build directory |
| `install` | Install dependencies only (no build) |
| `rebuild` | Clean and rebuild from scratch |
| `all` | Build + test + coverage |

---

## Build Options

All commands support these options:

| Option | Description |
|--------|-------------|
| `--debug` | Build in Debug mode (with symbols) |
| `--release` | Build in Release mode (optimized, default) |
| `--clean` | Clean before building |
| `--verbose` | Show verbose build output |
| `--jobs N` / `-j N` | Number of parallel jobs (default: CPU cores) |
| `--help` / `-h` | Show help message |

**Examples:**

```bash
# Debug build
./r-type.sh build --debug

# Clean rebuild with 8 parallel jobs
./r-type.sh rebuild -j 8

# Verbose Debug build
./r-type.sh build --debug --verbose
```

---

## 🔧 Manual Build (Advanced)

If you prefer to build manually without the unified script:

### Prerequisites

- **CMake** 3.20+
- **Python 3.7+** with pip
- **C++20 compiler** (GCC 10+, Clang 12+, MSVC 2019+)

### Step 1: Install Conan

```bash
pip install conan
conan profile detect --force
```

### Step 2: Install Dependencies

```bash
conan install . --output-folder=build --build=missing -s build_type=Release
```

This downloads and builds:
- **SFML 2.6.1** (graphics, window, audio)
- **Asio 1.30.2** (networking)
- **GTest 1.14.0** (testing)
- **LZ4 1.9.4** (compression)

### Step 3: Configure CMake

```bash
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```

### Step 4: Build

```bash
cmake --build . -j$(nproc)
```

### Step 5: Run

```bash
# Server
./bin/r-type_server

# Client (in another terminal)
./bin/r-type_client

# Admin client (optional)
./bin/r-type_admin

# Run tests
./bin/test_registry
./bin/test_collision
# ... or use ctest to run all
ctest --output-on-failure
```

---

## Build Types

### Debug Build

For development with debugging symbols:

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

**Features:**
- Debug symbols included
- Assertions enabled
- No optimizations
- Larger binary size
- Slower runtime

### Release Build

For production with optimizations:

```bash
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

**Features:**
- Full optimizations (-O3)
- No debug symbols
- Smaller binary size
- Maximum performance

### RelWithDebInfo Build

Best of both worlds:

```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

**Features:**
- Optimizations enabled
- Debug symbols included
- Good for profiling

## Build Options

### Enable Tests

Tests are built by default. To run them:

```bash
./r-type.sh test

# Or manually
cd build
ctest --output-on-failure

# Run specific test
./bin/test_compression
./bin/test_weapon --gtest_filter="Weapon.ShootingCooldown"
```

**Test Coverage:**
- 57 test files
- 456 total tests
- Categories: ECS, game logic, network, admin, render, integration

### Enable Code Coverage

```bash
./r-type.sh coverage

# Or manually
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
cmake --build .
ctest
# Generate HTML report
gcovr -r .. --html --html-details -o coverage.html
```

### Custom Compiler

```bash
# Use Clang
cmake -DCMAKE_CXX_COMPILER=clang++ ..

# Use GCC
cmake -DCMAKE_CXX_COMPILER=g++ ..
```

### Install Location

```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
cmake --build .
sudo cmake --install .
```

## Platform-Specific Builds

=== "Linux"

    ```bash
    # Standard build
    cmake ..
    make -j$(nproc)
    
    # With specific compiler
    export CC=gcc-11
    export CXX=g++-11
    cmake ..
    make
    ```

=== "macOS"

    ```bash
    # Standard build
    cmake ..
    make -j$(sysctl -n hw.ncpu)
    
    # Universal binary (Intel + Apple Silicon)
    cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
    make
    ```

=== "Windows"

    ```powershell
    # Visual Studio 2019
    cmake -G "Visual Studio 16 2019" -A x64 ..
    cmake --build . --config Release
    
    # Ninja (faster builds)
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
    ninja
    ```

## CMake Options Reference

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release/RelWithDebInfo) |
| `BUILD_TESTS` | ON | Build unit tests (enabled by default) |
| `BUILD_DOCS` | OFF | Build documentation with MkDocs |
| `ENABLE_COVERAGE` | OFF | Enable code coverage (requires Debug build) |
| `USE_SANITIZERS` | OFF | Enable AddressSanitizer/UBSanitizer |
| `CMAKE_TOOLCHAIN_FILE` | - | Conan toolchain (required for dependencies) |

### Example with Multiple Options

```bash
cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_COVERAGE=ON \
  -DUSE_SANITIZERS=ON \
  -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
  ..
```

## Cross-Compilation

### Linux to Windows (MinGW)

R-TYPE uses Conan for dependencies, which simplifies cross-compilation:

```bash
# Install MinGW cross-compiler
sudo apt install mingw-w64

# Create Conan profile for Windows
conan profile detect --name windows-mingw

# Edit profile to use MinGW
# Then build
conan install . --output-folder=build-windows \
  --profile=windows-mingw --build=missing
cd build-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++
cmake --build .
```

**Note:** Cross-compilation is experimental and may require additional configuration.

## Troubleshooting Build Issues

### Conan Not Found After Installation

```bash
# Add pip bin directory to PATH
export PATH="$HOME/.local/bin:$PATH"

# Or use python module
python3 -m pip install conan

# Verify
conan --version
```

### CMake Cache Problems

```bash
# Clear CMake cache
rm -rf CMakeCache.txt CMakeFiles/

# Or clean entire build directory
cd ..
rm -rf build
mkdir build
cd build

# Reconfigure
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
```

### Conan Dependencies Fail to Build

```bash
# Update Conan
pip install --upgrade conan

# Clear Conan cache
conan remove "*" -c

# Rebuild dependencies
conan install . --output-folder=build --build=missing -s build_type=Release
```

### Dependency Not Found

```bash
# Ensure Conan toolchain is used
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake

# Check Conan installed dependencies
conan list "*"
```

### Compiler Version Too Old

```bash
# Linux - Install newer GCC
sudo apt install gcc-11 g++-11
export CC=gcc-11
export CXX=g++-11

# Detect new compiler in Conan
conan profile detect --force

# macOS - Update Xcode
xcode-select --install

# Windows - Use Visual Studio 2019+ or MinGW-w64
```

### SFML Build Fails on Linux

```bash
# Install SFML system dependencies
sudo apt install libx11-dev libxrandr-dev libxcursor-dev \
                 libxi-dev libudev-dev libgl1-mesa-dev \
                 libopenal-dev libvorbis-dev libflac-dev

# Rebuild
./r-type.sh rebuild
```

### Parallel Build

```bash
# Use all CPU cores
cmake --build . -j$(nproc)

# Or with make
make -j8  # Use 8 cores
```

### Verbose Build Output

```bash
# See full compilation commands
cmake --build . --verbose

# Or with make
make VERBOSE=1

# Or use the script
./r-type.sh build --verbose
```

### Build Fails on Windows

**Issue:** MinGW linker errors or missing DLLs

**Solution:**
1. Use MSYS2 MinGW-w64 (not standard MinGW)
2. Ensure MinGW bin directory is in PATH
3. Use Release build (Debug may have DLL issues)

```bash
# In MSYS2 MinGW 64-bit terminal
./r-type.sh build --release
```

### Tests Fail to Build

```bash
# Ensure GTest is installed via Conan
conan install . --output-folder=build --build=missing

# Check if test binaries exist
ls build/bin/test_*

# If missing, rebuild
./r-type.sh rebuild
```

## Build Scripts

The `r-type.sh` (Linux/macOS) and `r-type.bat` (Windows) scripts provide convenience commands:

```bash
# Quick builds
./r-type.sh build          # Release build
./r-type.sh build --debug  # Debug build
./r-type.sh rebuild        # Clean + build

# Run components
./r-type.sh server         # Build + run server
./r-type.sh client         # Build + run client
./r-type.sh admin          # Build + run admin client

# Testing & analysis
./r-type.sh test           # Run all 456 tests
./r-type.sh coverage       # Generate coverage report
./r-type.sh valgrind       # Memory leak detection

# Maintenance
./r-type.sh clean          # Clean build directory
./r-type.sh install        # Install dependencies only
```

**Script Features:**
- Auto-installs Conan if missing
- Parallel builds (uses all CPU cores)
- Colored output for readability
- Progress indicators
- Error handling and validation

## Continuous Integration

Our CI pipeline builds and tests for multiple platforms:

- ✅ **Linux** (Ubuntu 22.04) with GCC 11
- ✅ **macOS** (macOS 12) with Apple Clang
- ✅ **Windows** (Windows Server 2022) with MSVC 2022

**CI Workflow:**
1. Install dependencies via Conan
2. Build in Release mode
3. Run all 456 tests
4. Generate coverage report (Linux only)
5. Upload artifacts (binaries, coverage)

**Test Requirements:**
- All tests must pass (456/456)
- No memory leaks (Valgrind on Linux)
- Code coverage > 70% (target)

See `.github/workflows/ci.yml` for full CI configuration.

**Branch Protection:**
- `main` branch requires passing CI
- Pull requests must pass all checks
- Code review required before merge

## Next Steps

- 🧪 [Testing Guide](../developer-guide/testing.md) - Learn about the test suite
- � [Quick Start](quickstart.md) - Run and play the game
- 🏗️ [Architecture Overview](../architecture/overview.md) - Understand the codebase
- 🌐 [Network Documentation](../architecture/network.md) - Network protocol details
- 🤝 [Contributing Guide](../developer-guide/contributing.md) - Contribute to the project
