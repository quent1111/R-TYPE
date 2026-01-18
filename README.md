# R-Type

![CI/CD](https://img.shields.io/github/actions/workflow/status/quent1111/R-TYPE/ci.yml?branch=main&label=CI/CD)
![Tests](https://img.shields.io/github/actions/workflow/status/quent1111/R-TYPE/ci.yml?branch=main&label=tests)
![Coverage](https://img.shields.io/github/actions/workflow/status/quent1111/R-TYPE/coverage.yml?branch=main&label=coverage)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

A modern, networked reimplementation of the classic R-Type shoot'em up game, built from scratch with a custom Entity-Component-System game engine.

[Documentation](https://quent1111.github.io/r-type) • [Report Bug](https://github.com/quent1111/r-type/issues) • [Request Feature](https://github.com/quent1111/r-type/issues)

---

## About

R-Type is a networked multiplayer implementation of the classic horizontal shoot'em up game. This project demonstrates advanced software architecture, networked game development, and modern C++ engineering practices.

**Key Technologies:**
- Custom Entity-Component-System (ECS) game engine
- UDP-based client-server networking with binary protocol
- Cross-platform support (Linux, macOS, Windows)
- Modern C++20 with best practices

Developed as part of the Advanced C++ curriculum at Epitech.

---

## Features

### Gameplay
- 4-player cooperative multiplayer
- Progressive level system with increasing difficulty
- Multiple enemy types with unique behaviors
- Power-up system (speed boost, shield, damage multiplier)
- Boss encounters with multi-phase mechanics
- Score tracking and combo system
- Real-time visual effects (explosions, particles, screen shake)

### Technical Implementation
- **ECS Architecture**: Decoupled component-based game engine
- **Network Protocol**: Custom binary UDP protocol with entity synchronization
- **Server Authority**: Server-side game logic and collision detection
- **Client Rendering**: SFML-based rendering with parallax backgrounds
- **Audio System**: Background music and positional sound effects
- **State Management**: Menu system with settings persistence
- **Input Handling**: Keyboard and mouse support with configurable controls

### Engineering
- CMake build system with Conan package management
- Comprehensive test suite (network, game logic, client components)
- CI/CD pipeline with automated testing and linting
- Cross-platform compatibility layer
- Documentation with Doxygen and MkDocs

---

## Requirements

### System Requirements

**Supported Platforms:**
- Linux (Ubuntu 20.04+, Debian 11+, or equivalent)
- macOS (11.0+ Big Sur or later)
- Windows (10/11 with MinGW-w64 or MSVC)

**Build Tools:**
- CMake 3.20 or higher
- C++20 compatible compiler:
  - GCC 10+ (Linux)
  - Clang 12+ (macOS)
  - MSVC 2019+ or MinGW-w64 GCC 10+ (Windows)
- Python 3.7+ (for Conan package manager)
- pip (Python package installer)

**Runtime Dependencies:**
- SFML 2.6.1 (automatically installed via Conan)
- Asio 1.30.2 (automatically installed via Conan)
- LZ4 1.9.4 (automatically installed via Conan)

### Installing Prerequisites

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install build-essential cmake python3 python3-pip git
```

**macOS:**
```bash
brew install cmake python git
# For ARM64 Macs, also install SFML via Homebrew:
brew install sfml
```

**Windows:**

Option 1 - MinGW-w64 via MSYS2 (recommended):
1. Download and install MSYS2 from https://www.msys2.org/
2. Open **MSYS2 UCRT64** terminal (not MSYS2 or MinGW64)
3. Update packages: `pacman -Syu`
4. Install tools: `pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make`
5. Add `C:\msys64\ucrt64\bin` to your Windows PATH:
   - **Temporary (current session)**: In PowerShell: `$env:Path += ";C:\msys64\ucrt64\bin"`
   - **Permanent**: System Properties → Environment Variables → Path → Add `C:\msys64\ucrt64\bin`
6. Restart PowerShell and verify: `g++ --version`

Option 2 - Visual Studio:
1. Install Visual Studio 2019+ with C++ development tools
2. Install CMake from https://cmake.org/download/
3. Install Python from https://www.python.org/downloads/

**Python Dependencies (for documentation):**
```bash
pip install -r docs/requirements.txt
```

---

## Quick Start

### Building and Running

**Start the server:**
```bash
./r-type.sh server        # Linux/macOS
r-type.bat server         # Windows
```

**Start the client:**
```bash
./r-type.sh client        # Linux/macOS
r-type.bat client         # Windows
```

**Start the admin panel:**
```bash
./r-type.sh admin         # Linux/macOS
r-type.bat admin          # Windows
```
Default admin password: `admin123`

The build script automatically:
- Installs Conan package manager if needed
- Downloads and builds dependencies (SFML, Asio, GTest)
- Configures CMake with appropriate toolchain
- Builds the project in parallel
- Launches the executable

### Available Commands

| Command | Description |
|---------|-------------|
| `./r-type.sh build` | Build the entire project |
| `./r-type.sh client` | Build and run the client |
| `./r-type.sh server` | Build and run the server |
| `./r-type.sh admin` | Build and run the admin panel |
| `./r-type.sh test` | Run all tests via CTest |
| `./r-type.sh tests` | Run game unit tests directly |
| `./r-type.sh coverage` | Generate code coverage report |
| `./r-type.sh clean` | Clean build directory |
| `./r-type.sh rebuild` | Clean and rebuild from scratch |

**Build Options:** `--debug`, `--release`, `--clean`, `--verbose`, `-j N`

Full build documentation: [BUILD.md](BUILD.md)

---

## Gameplay

### Controls
- **Arrow Keys**: Move spaceship (Up, Down, Left, Right)
- **Space**: Fire projectiles
- **1/2**: Select power-up when prompted
- **Escape**: Pause / Return to menu

### Game Mechanics
- Destroy enemies to progress through levels
- Each level requires defeating a specific number of enemies
- Collect power-ups to enhance your abilities:
  - **Speed Boost**: Increased movement speed
  - **Shield**: Temporary invulnerability
  - **Damage Multiplier**: Increased weapon damage
- Level 5 features a boss encounter with multiple phases
- Score increases with enemy kills and combo multipliers

### Enemy Types
- **Standard Enemy**: Basic horizontal movement, shoots straight projectiles
- **Advanced Enemy**: Diagonal movement, fires homing projectiles
- **Boss**: Multi-phase encounter with multiple hitboxes and attack patterns

---

## Documentation

- [Architecture Overview](./docs/architecture/overview.md): System design and component interaction
- [ECS Pattern](./docs/architecture/ecs-pattern.md): Entity-Component-System implementation
- [Network Protocol RFC](./docs/api/protocol/rfc.md): Complete protocol specification
- [Developer Guide](./docs/development/setup.md): Setup and contribution guidelines

Full documentation: [https://quent1111.github.io/r-type](https://quent1111.github.io/r-type)

---

## Contributing

Contributions are welcome. Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/feature-name`)
3. Follow the C++ Core Guidelines
4. Write unit tests for new features
5. Use `./r-type.sh format` for code formatting
6. Update documentation as needed
7. Submit a pull request

See [CONTRIBUTING.md](./CONTRIBUTING.md) for detailed guidelines.

---



