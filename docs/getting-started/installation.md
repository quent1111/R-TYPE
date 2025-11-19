# Installation Guide

This guide will help you install and set up the R-TYPE project on your system.

## Prerequisites

### Required Tools

- **C++ Compiler** with C++17 support
  - GCC 7+ or Clang 5+ (Linux)
  - MSVC 2017+ (Windows)
  - Clang 5+ (macOS)
- **CMake** 3.15 or higher
- **Git** for version control

### Development Tools (Recommended)

- **clang-format** - For code formatting
- **clang-tidy** - For static analysis
- **Python 3.8+** - For documentation and scripts

## Platform-Specific Setup

=== "Linux (Ubuntu/Debian)"

    ```bash
    # Update package list
    sudo apt update
    
    # Install build essentials
    sudo apt install build-essential cmake git
    
    # Install development tools
    sudo apt install clang-format clang-tidy
    
    # Install SFML dependencies (if using SFML)
    sudo apt install libsfml-dev
    ```

=== "macOS"

    ```bash
    # Install Homebrew if not already installed
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    
    # Install required tools
    brew install cmake git
    brew install llvm  # For clang-format and clang-tidy
    
    # Install SFML (if using SFML)
    brew install sfml
    ```

=== "Windows"

    1. **Install Visual Studio 2019 or later**
       - Download from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
       - Select "Desktop development with C++" workload
    
    2. **Install CMake**
       - Download from [cmake.org](https://cmake.org/download/)
       - Add to PATH during installation
    
    3. **Install Git**
       - Download from [git-scm.com](https://git-scm.com/)

## Cloning the Repository

```bash
# Clone the repository
git clone https://github.com/quent1111/R-TYPE.git

# Navigate to the project directory
cd R-TYPE

# Install git hooks for code quality
./scripts/install-hooks.sh
```

## Building the Project

### Using CMake (Recommended)

```bash
# Create build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build the project
cmake --build .

# Or use make directly on Unix systems
make
```

### Build Options

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# With tests
cmake -DBUILD_TESTS=ON ..
```

## Verifying Installation

After building, verify that everything works:

```bash
# Run tests (if built with tests)
ctest

# Run the server
./r-type_server

# In another terminal, run the client
./r-type_client
```

## Next Steps

- 📖 [Quick Start Guide](quickstart.md) - Learn how to use R-TYPE
- 🏗️ [Building Guide](building.md) - Advanced build configurations
- 🤝 [Contributing](../developer-guide/contributing.md) - Start contributing

## Troubleshooting

### CMake can't find dependencies

Make sure all required libraries are installed. On Linux:

```bash
sudo apt install libsfml-dev
```

### Compiler errors about C++17

Ensure your compiler supports C++17:

```bash
# Check GCC version
g++ --version  # Should be 7.0 or higher

# Check Clang version
clang++ --version  # Should be 5.0 or higher
```

### Git hooks not working

Ensure the install script has execute permissions:

```bash
chmod +x scripts/install-hooks.sh
./scripts/install-hooks.sh
```

## Need Help?

- 📝 Check the [FAQ](../faq.md)
- 🐛 [Report an issue](https://github.com/quent1111/R-TYPE/issues)
- 💬 Join our Discord community
