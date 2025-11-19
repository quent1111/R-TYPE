# R-Type

![CI/CD](https://img.shields.io/github/actions/workflow/status/quent1111/R-TYPE/ci.yml?branch=main&label=CI/CD)
![Tests](https://img.shields.io/github/actions/workflow/status/quent1111/R-TYPE/ci.yml?branch=main&label=tests)
![Coverage](https://img.shields.io/github/actions/workflow/status/quent1111/R-TYPE/coverage.yml?branch=main&label=coverage)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

> A modern, networked reimplementation of the classic R-Type shoot'em up game, built from scratch with a custom Entity-Component-System game engine.

[📚 Documentation](https://quent1111.github.io/r-type) • [🐛 Report Bug](https://github.com/quent1111/r-type/issues) • [✨ Request Feature](https://github.com/quent1111/r-type/issues)

---

## 📖 About

R-Type is a networked multiplayer implementation of the classic horizontal shoot'em up game. This project features:

- **Custom Game Engine**: Built from scratch using the Entity-Component-System (ECS) architectural pattern
- **Multiplayer Networking**: UDP-based client-server architecture supporting up to 4 simultaneous players
- **Cross-Platform**: Runs on both Linux and Windows with seamless cross-play
- **Modern C++**: Written in C++20 with best practices and design patterns

This project was developed as part of the Advanced C++ curriculum at Epitech, demonstrating advanced software architecture, networked game development, and professional software engineering practices.

---

## ✨ Features

### Core Gameplay
- 🎮 **4-Player Co-op**: Fight together against waves of Bydo enemies
- 🚀 **Classic Mechanics**: Player spaceships, missiles, and enemy AI
- 💥 **Real-time Combat**: Smooth 60 FPS gameplay with collision detection
- 🏆 **Score System**: Compete for the highest score

### Technical Features
- 🏗️ **ECS Architecture**: Decoupled, reusable game engine components
- 🌐 **UDP Networking**: Low-latency binary protocol for real-time gameplay
- 🔀 **Multithreaded Server**: Non-blocking game logic and network I/O
- 🎨 **Parallax Scrolling**: Multi-layer starfield background
- 🔊 **Audio System**: Background music and sound effects
- 📊 **Visual UI**: Health bars, scores, and player identification

### Engineering
- 🔧 **CMake Build System**: Modern, cross-platform build configuration
- 📦 **Package Management**: Conan/Vcpkg for dependency management
- 🤖 **CI/CD Pipeline**: Automated builds, tests, and linting
- 📝 **Comprehensive Documentation**: Architecture, API, and protocol specs
- ✅ **Unit & Integration Tests**: High code coverage with automated testing

---

## 🚀 Quick Start

### ⚡ Ultra-Fast Start (Recommended)

```bash
# Linux/macOS
./r-type.sh server

# Windows
r-type.bat server
```

**That's it!** The script automatically:
- ✅ Installs Conan package manager
- ✅ Downloads and builds dependencies (SFML, Asio, GTest)
- ✅ **Auto-installs system packages** (X11 libraries on Linux) - *may ask for sudo password*
- ✅ Configures CMake with the right toolchain
- ✅ Builds the server in parallel
- ✅ Launches the server


**In another terminal, run the client:**
```bash
# Linux/macOS
./r-type.sh client

# Windows
r-type.bat client
```

### 📖 All Available Commands

| Command | Description |
|---------|-------------|
| `./r-type.sh build` | Build the entire project |
| `./r-type.sh client` | Build and run the client |
| `./r-type.sh server` | Build and run the server |
| `./r-type.sh test` | Run all tests |
| `./r-type.sh coverage` | Generate code coverage (Linux/macOS) |
| `./r-type.sh clean` | Clean build directory |
| `./r-type.sh rebuild` | Clean and rebuild from scratch |

**Options:** `--debug`, `--release`, `--clean`, `--verbose`, `-j N`

📚 **Full build documentation:** [BUILD.md](BUILD.md)

---

## 🎮 How to Play

### Controls
- **Arrow Keys**: Move your spaceship (Up, Down, Left, Right)
- **Spacebar**: Shoot missiles
- **ESC**: Pause / Disconnect

### Objective
Survive waves of Bydo enemies, shoot them down, and achieve the highest score!

---

## 🏗️ Architecture

### High-Level Overview
```
┌─────────────────────────────────────────┐
│            Game Engine (ECS)            │
│  ┌──────────┬──────────┬──────────┐     │
│  │ Entities │Components│ Systems  │     │
│  └──────────┴──────────┴──────────┘     │
└─────────────────────────────────────────┘
         │                      │
    ┌────┴────┐            ┌────┴────┐
    │ Client  │            │ Server  │
    ├─────────┤            ├─────────┤
    │Rendering│            │ Game    │
    │  Input  │◄──UDP────► │ Logic   │
    │ Audio   │            │Network  │
    └─────────┘            └─────────┘
```

### Key Components
- **ECS Core**: Entity-Component-System pattern for flexible game object management
- **Network Layer**: Binary UDP protocol with client-server architecture
- **Rendering Engine**: SFML-based sprite rendering with parallax scrolling
- **Game Logic**: Server-authoritative gameplay with collision detection

📚 For detailed architecture documentation, see [docs/architecture/](./docs/architecture/)

---

## 📡 Network Protocol

The game uses a custom binary UDP protocol for low-latency communication.

**Key Message Types:**
- `CONNECT` / `DISCONNECT`: Connection management
- `PLAYER_INPUT`: Client input updates
- `ENTITY_SPAWN` / `ENTITY_UPDATE` / `ENTITY_DESTROY`: Game state synchronization

📄 Full protocol specification: [docs/api/protocol/rfc.md](./docs/api/protocol/rfc.md)

---

## 🧪 Testing
```bash
# Run all tests
cd build
ctest -C Release --output-on-failure

# Run specific test suite
./tests/engine_tests
./tests/network_tests

# Generate coverage report (Linux only)
cmake --build . --target coverage
```

**Current Coverage:** `TODO: XX%`

---

## 📚 Documentation

- **[Architecture Overview](./docs/architecture/overview.md)**: System design and component interaction
- **[ECS Pattern](./docs/architecture/ecs-pattern.md)**: Entity-Component-System implementation
- **[Network Protocol RFC](./docs/api/protocol/rfc.md)**: Complete protocol specification
- **[Developer Guide](./docs/development/setup.md)**: Setup and contribution guidelines
- **[Comparative Study](./docs/comparative-study/)**: Technology choices and justifications
- **[API Reference](https://quent1111.github.io/r-type/api/)**: Doxygen-generated API docs

🌐 **Full documentation:** [https://quent1111.github.io/r-type](https://quent1111.github.io/r-type)

---

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](./CONTRIBUTING.md) for details.

### Development Workflow
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Standards
- Follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- Use `clang-format` for code formatting (config provided)
- Write unit tests for new features
- Update documentation as needed

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

---

## 🙏 Acknowledgments

- **R-Type** by Irem (1987) - Original game inspiration
- **[SFML](https://www.sfml-dev.org/)** - Graphics and Audio library
- **[Asio](https://think-async.com/Asio/)** - Cross-platform networking
- **Epitech** - Advanced C++ curriculum and project framework
- **[Sprites Resource](https://www.spriters-resource.com/)** - R-Type sprite assets

---

[⬆ Back to Top](#r-type)

</div>
