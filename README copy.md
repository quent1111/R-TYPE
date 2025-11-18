# R-TYPE
**< A GAME ENGINE THAT ROARS ! />**

[![Build Status](https://github.com/EpitechPGE3-2025/G-CPP-500-NAN-5-2-rtype-4/workflows/Build%20&%20Test/badge.svg)](https://github.com/EpitechPGE3-2025/G-CPP-500-NAN-5-2-rtype-4/actions)

A networked multiplayer R-Type clone built with a custom C++ game engine featuring Entity-Component-System architecture, UDP networking, and cross-platform support.

## 📋 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Documentation](#documentation)
- [Contributing](#contributing)

## 🎮 Overview

This project implements a multiplayer R-Type game with:
- **Custom game engine** with ECS architecture
- **Client-server networking** using UDP protocol
- **Multi-threaded authoritative server**
- **Cross-platform** (Linux & Windows)
- **Modern C++17** codebase

## ✨ Features

### Engine
- **ECS (Entity-Component-System)**: Flexible component-based architecture
- **Networking**: UDP-based protocol with optional TCP for reliable messages
- **Rendering**: SFML-based 2D graphics with sprite management
- **Audio**: Sound effects and background music support
- **Physics**: Basic collision detection and movement

### Gameplay
- **Multiplayer**: Up to 4 players in co-op mode
- **Enemies**: Bydos monsters with various patterns
- **Weapons**: Player missiles and power-ups
- **Scrolling**: Horizontal starfield background

## 🏗️ Architecture

```
r-type/
├── engine/          # Game engine (reusable library)
│   ├── ecs/        # Entity-Component-System
│   ├── net/        # Networking layer (UDP/TCP)
│   ├── render/     # Rendering abstractions
│   ├── audio/      # Audio subsystem
│   ├── core/       # Core utilities (time, logging)
│   └── utils/      # Math, geometry helpers
├── client/         # Client application
│   ├── src/        # Client-specific code
│   ├── ui/         # UI components
│   └── input/      # Input handling
├── server/         # Server application
│   ├── src/        # Server-specific code
│   ├── game_logic/ # R-Type game logic
│   └── instances/  # Multi-instance management
├── tests/          # Unit and integration tests
├── docs/           # Documentation
├── assets/         # Game assets (sprites, sounds)
├── tools/          # Editors and utilities
└── examples/       # Example games using the engine
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed design documentation.

## 📦 Requirements

### System
- **OS**: Linux (Ubuntu 20.04+) or Windows 10/11
- **Compiler**: GCC 9+ / Clang 10+ / MSVC 2019+
- **CMake**: 3.18 or higher
- **Python**: 3.7+ (for Conan)

### Dependencies (auto-installed via Conan)
- **SFML**: 2.6.1 (graphics, audio, window, network)
- **Asio**: 1.28.0 (async networking)
- **GoogleTest**: 1.14.0 (unit tests)

## 🔧 Build Instructions

### Linux (recommended)

1. **Install system dependencies**:
   ```bash
   sudo apt-get update
   sudo apt-get install -y cmake g++ python3-pip git
   ```

2. **Install Conan package manager**:
   ```bash
   pip3 install conan
   conan profile detect --force
   ```

3. **Clone the repository**:
   ```bash
   git clone https://github.com/EpitechPGE3-2025/G-CPP-500-NAN-5-2-rtype-4.git
   cd G-CPP-500-NAN-5-2-rtype-4
   ```

4. **Install dependencies with Conan**:
   ```bash
   conan install . --output-folder=build --build=missing
   ```

5. **Configure and build**:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
   cmake --build build --config Release -j$(nproc)
   ```

6. **Run tests** (optional):
   ```bash
   cd build && ctest --output-on-failure
   ```

### Windows (MSVC)

1. Install Visual Studio 2019+ with C++ Desktop Development
2. Install Python 3.7+ and Conan: `pip install conan`
3. Open PowerShell in project root:
   ```powershell
   conan profile detect --force
   conan install . --output-folder=build --build=missing
   cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
   cmake --build build --config Release
   ```

### Alternative: Using vcpkg

If you prefer vcpkg over Conan, create `vcpkg.json`:
```json
{
  "dependencies": ["sfml", "asio", "gtest"]
}
```
Then build with: `cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake`

## 🚀 Usage

### Running the Server

```bash
./build/bin/r-type_server --port 4242
```

Options:
- `--port <N>`: UDP port (default: 4242)
- `--max-players <N>`: Max players per instance (default: 4)
- `--tick-rate <N>`: Simulation rate in Hz (default: 60)

### Running the Client

```bash
./build/bin/r-type_client --server 127.0.0.1 --port 4242
```

Options:
- `--server <IP>`: Server address (default: 127.0.0.1)
- `--port <N>`: Server port (default: 4242)
- `--name <NAME>`: Player name (default: "Player")

### Controls

| Key          | Action              |
|--------------|---------------------|
| Arrow Keys   | Move spaceship      |
| Space        | Fire missile        |
| ESC          | Pause / Menu        |

## 📚 Documentation

- **[Architecture Guide](ARCHITECTURE.md)**: Design principles and module overview
- **[Protocol Specification](docs/protocol.md)**: Network protocol RFC
- **Developer Docs**: See `docs/` folder for API references and tutorials

## 🧪 Testing

Run all tests:
```bash
cd build
ctest --output-on-failure
```

Run specific test suite:
```bash
./build/tests/ecs_tests
./build/tests/net_tests
```

## 🛠️ Development

### Code Style
- Follow Google C++ Style Guide
- Use `clang-format` (config in `.clang-format`)
- Run linter: `clang-tidy src/**/*.cpp`

### Git Workflow
1. Create feature branch: `git checkout -b feature/my-feature`
2. Commit with descriptive messages
3. Open Pull Request to `main`
4. CI must pass before merge

### CI/CD
GitHub Actions automatically:
- Builds on Linux (Ubuntu latest)
- Runs all tests
- Checks for compilation warnings

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

See [CONTRIBUTING.md](docs/CONTRIBUTING.md) for detailed guidelines.

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file.

## 👥 Authors

- **R-Type Team** - Epitech Nancy 2025

## 🙏 Acknowledgments

- Original R-Type by Irem (1987)
- SFML library contributors
- Epitech pedagogical team

---

**For detailed architecture, design decisions, and technical specifications, please refer to [ARCHITECTURE.md](ARCHITECTURE.md) and the [docs/](docs/) folder.**
