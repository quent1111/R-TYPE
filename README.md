# R-TYPE

```
< A GAME ENGINE THAT ROARS ! />
```

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20+-blue.svg)](https://cmake.org/)

A modern C++ implementation of the classic R-Type game, featuring a networked multiplayer architecture and a custom game engine built from scratch.

## 📋 Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Documentation](#documentation)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)
- [Authors](#authors)

## ✨ Features

- 🎮 **Networked Multiplayer** - Up to 4 players over UDP
- 🚀 **Custom Game Engine** - ECS architecture with modular subsystems
- 🎨 **Cross-Platform** - Runs on Linux and Windows
- 🔧 **Modern C++20** - Clean, maintainable codebase
- 📦 **Self-Contained** - All dependencies managed via Conan
- 🧪 **Fully Tested** - Unit tests with GoogleTest

## 🚀 Quick Start

### Prerequisites

- **CMake** 3.20+
- **C++20 compiler** (GCC 9+, Clang 10+, or MSVC 2019+)
- **Python 3.7+** (for Conan package manager)

### Build in 3 Simple Commands

```bash
# 1. Clone the repository
git clone https://github.com/quent1111/R-TYPE.git
cd R-TYPE

# 2. Install Conan (one-time setup)
pip install conan

# 3. Run the build script (it handles EVERYTHING automatically!)
./scripts/build.sh
```

**What the script does automatically:**
- ✅ Creates Conan profile if missing
- ✅ Checks and installs system dependencies (X11 libs on Linux)
- ✅ Downloads and compiles SFML, Asio, GoogleTest
- ✅ Configures CMake with C++20
- ✅ Compiles server, client, and tests

**First build:** 5-10 minutes (compiles all libraries)  
**Subsequent builds:** 10-30 seconds (everything cached!)

### Running the Game

**Start the server:**
```bash
cd build/bin
./r-type_server
```

**Start the client (in another terminal):**
```bash
cd build/bin
./r-type_client
```

**Run tests:**
```bash
cd build
ctest --output-on-failure
```

### Windows Users

Use `scripts/build.bat` instead:
```cmd
scripts\build.bat
```

## 📚 Documentation

Comprehensive documentation is available in the `docs/` directory:

- **[Development Setup Guide](docs/development/setup.md)** - Detailed installation and troubleshooting
- **[Network Protocol](docs/protocol.md)** - Binary protocol specification (RFC-style)
- **[Contributing Guide](docs/CONTRIBUTING.md)** - Development workflow and conventions

## 📁 Project Structure

```
r-type/
├── engine/              # Game engine (ECS, networking, rendering, audio)
│   ├── ecs/            # Entity-Component-System
│   ├── net/            # Networking layer (UDP/TCP)
│   ├── render/         # Rendering abstractions (SFML)
│   ├── audio/          # Audio subsystem
│   ├── core/           # Core utilities (time, logging)
│   └── utils/          # Math, geometry helpers
├── server/             # Authoritative game server
├── client/             # Graphical client application
├── tests/              # Unit and integration tests
├── docs/               # Project documentation
├── assets/             # Game assets (sprites, sounds, fonts)
├── scripts/            # Build and automation scripts
└── CMakeLists.txt      # Root CMake configuration
```

## 🔧 Building Manually

If you prefer manual control over the build process:

```bash
# Install dependencies
conan install . --output-folder=build --build=missing

# Configure CMake
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Run tests
ctest --output-on-failure
```

## 🧪 Running Tests

```bash
cd build
ctest --output-on-failure
```

## 🛠️ Technologies Used

- **Language:** C++20
- **Build System:** CMake 3.20+
- **Package Manager:** Conan 2.x
- **Graphics:** SFML 2.6
- **Networking:** Asio (standalone)
- **Testing:** GoogleTest 1.14

## 🤝 Contributing

We welcome contributions! Please read our [Contributing Guide](docs/CONTRIBUTING.md) for details on:

- Development workflow
- Code style conventions
- Git practices
- Pull request process

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Authors

**Team R-Type - Epitech Lyon 2025**

- [@quent1111](https://github.com/quent1111)
- [@djellon](https://github.com/djellon)
- [Add other team members]

---

**Made with ❤️ at Epitech Lyon**