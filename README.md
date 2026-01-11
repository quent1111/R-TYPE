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



