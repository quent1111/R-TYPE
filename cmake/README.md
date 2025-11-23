# CMake Modules

This directory contains reusable CMake modules for the R-Type project.

## Available Modules

### `CompilerOptions.cmake`
Configures compiler-specific options and optimizations for cross-platform builds.

**Features:**
- Sets C++20 standard
- Enables colored diagnostics (GCC/Clang)
- Platform-specific compiler flags (MSVC, GCC, Clang)
- Build type configurations (Debug, Release, RelWithDebInfo)

**Usage:**
```cmake
include(CompilerOptions)
```

---

### `CompilerWarnings.cmake`
Sets up strict compiler warnings to catch potential issues early.

**Features:**
- MSVC warnings (W4 level with specific flags)
- GCC/Clang warnings (Wall, Wextra, and more)
- Optional "warnings as errors" mode

**Usage:**
```cmake
include(CompilerWarnings)

# Apply warnings to a target
add_library(my_target ...)
set_project_warnings(my_target)
```

**Enable warnings as errors:**
```cmake
set(WARNINGS_AS_ERRORS ON)
```

---

### `Platform.cmake`
Platform detection and helper functions for cross-platform development.

**Features:**
- Automatic platform detection (Windows, Linux, macOS)
- Platform-specific variables (`PLATFORM_WINDOWS`, `PLATFORM_LINUX`, etc.)
- Helper functions for platform-specific sources and libraries

**Usage:**
```cmake
include(Platform)

# Platform detection
if(PLATFORM_WINDOWS)
    # Windows-specific code
endif()

# Link platform-specific libraries (ws2_32 on Windows, pthread on Linux)
link_platform_libraries(my_target)
```

---

## Integration

All modules are automatically included in the root `CMakeLists.txt`:

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Platform)
include(CompilerOptions)
include(CompilerWarnings)
```

---

## Adding New Modules

To add a new CMake module:

1. Create a new `.cmake` file in this directory
2. Include it in the root `CMakeLists.txt`
3. Document it in this README

---

## Platform-Specific Notes

### Windows (MSVC)
- Multi-processor compilation enabled (`/MP`)
- UTF-8 source and execution character sets
- Dynamic runtime library (MultiThreadedDLL)

### Linux/macOS (GCC/Clang)
- pthread support enabled
- Colored diagnostics
- Position-independent code for shared libraries

---

## Build Types

| Build Type | Optimization | Debug Info | Use Case |
|------------|--------------|------------|----------|
| **Debug** | None (`-O0`) | Full (`-g3`) | Development and debugging |
| **Release** | Maximum (`-O3`) | None | Production builds |
| **RelWithDebInfo** | Moderate (`-O2`) | Yes (`-g`) | Profiling and release testing |

---

## References

- [CMake Documentation](https://cmake.org/documentation/)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
- [Effective Modern CMake](https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1)
