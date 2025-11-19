# Building R-TYPE

Advanced build configurations and options for R-TYPE.

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

```bash
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest  # Run tests
```

### Enable Code Coverage

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
cmake --build .
ctest
# Generate coverage report
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
| `BUILD_TESTS` | OFF | Build unit tests |
| `BUILD_DOCS` | OFF | Build documentation |
| `ENABLE_COVERAGE` | OFF | Enable code coverage |
| `USE_SANITIZERS` | OFF | Enable address/UB sanitizers |
| `BUILD_SHARED_LIBS` | OFF | Build shared libraries |
| `WARNINGS_AS_ERRORS` | OFF | Treat warnings as errors |

### Example with Multiple Options

```bash
cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON \
  -DUSE_SANITIZERS=ON \
  -DWARNINGS_AS_ERRORS=ON \
  ..
```

## Cross-Compilation

### Linux to Windows (MinGW)

```bash
cmake \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  ..
```

## Troubleshooting Build Issues

### CMake Cache Problems

```bash
# Clear CMake cache
rm -rf CMakeCache.txt CMakeFiles/

# Or clean entire build directory
cd ..
rm -rf build
mkdir build
cd build
cmake ..
```

### Dependency Not Found

```bash
# Tell CMake where to find dependencies
cmake -DSFML_DIR=/path/to/sfml/lib/cmake/SFML ..
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
```

## Build Scripts

For convenience, you can use our build scripts:

```bash
# Quick debug build
./scripts/build.sh debug

# Release build
./scripts/build.sh release

# Clean and rebuild
./scripts/build.sh clean
```

## Continuous Integration

Our CI pipeline builds for multiple platforms:

- ✅ Linux (GCC, Clang)
- ✅ macOS (Apple Clang)
- ✅ Windows (MSVC)

See `.github/workflows/` for CI configuration.

## Next Steps

- 🧪 [Testing Guide](../developer-guide/testing.md)
- 📦 [Deployment](../guides/deployment.md)
- 🏗️ [Architecture](../architecture/overview.md)
