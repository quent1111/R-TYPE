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

## Architecture

### System Overview
```
┌─────────────────────────────────────────┐
│         Game Engine (ECS Core)          │
│  ┌──────────┬───────────┬──────────┐    │
│  │ Registry │ Components│ Systems  │    │
│  │ (Entity  │ (Data)    │ (Logic)  │    │
│  │  Manager)│           │          │    │
│  └──────────┴───────────┴──────────┘    │
└─────────────────────────────────────────┘
              │                 │
     ┌────────┴────────┐   ┌────┴─────────┐
     │     Client      │   │    Server    │
     ├─────────────────┤   ├──────────────┤
     │ - Rendering     │   │ - Game Loop  │
     │ - Input         │   │ - Physics    │
     │ - Audio         │   │ - Collision  │
     │ - UI/Menus      │   │ - AI         │
     │ - Effects       │   │ - Spawning   │
     └─────────────────┘   └──────────────┘
              │                 │
              └────── UDP ──────┘
           Binary Protocol (60Hz)
```

### Core Components

**ECS Engine:**
- `registry`: Central entity and component manager
- `sparse_array`: Efficient component storage
- `entity`: Type-safe entity handles
- `zipper`: Parallel component iteration

**Game Systems:**
- `movement_system`: Entity position updates
- `collision_system`: AABB collision detection
- `shooting_system`: Projectile spawning and AI
- `health_system`: Damage processing and death handling
- `wave_system`: Enemy spawning and level progression
- `boss_system`: Boss behavior and phase transitions

**Network Layer:**
- Binary serializer with efficient packing
- Client-server synchronization (entity spawn/update/destroy)
- Input buffering and prediction
- Connection management and heartbeat

**Client Systems:**
- State machine (Menu, Lobby, Game)
- Texture and font management
- Audio playback with spatial effects
- Visual effects (particles, screen shake, animations)

Full architecture documentation: [docs/architecture/](./docs/architecture/)

---

## Network Protocol

The game uses a custom binary UDP protocol optimized for low-latency real-time communication.

**Packet Structure:**
```
[Magic Number (2 bytes)][OpCode (1 byte)][Payload (variable)]
```

**Key OpCodes:**
- Connection: `CONNECT`, `DISCONNECT`, `HEARTBEAT`
- Game State: `ENTITY_SPAWN`, `ENTITY_UPDATE`, `ENTITY_DESTROY`
- Player: `PLAYER_INPUT`, `PLAYER_SHOOT`
- Level: `LEVEL_START`, `LEVEL_COMPLETE`
- Power-ups: `POWERUP_SELECTION`, `POWERUP_CHOICE`, `POWERUP_STATUS`

**Entity Types:**
- `0x01`: Player
- `0x02`: Standard Enemy
- `0x03`: Projectile
- `0x04`: Power-up
- `0x05`: Obstacle
- `0x06`: Advanced Enemy
- `0x08`: Boss

Full protocol specification: [docs/api/protocol/rfc.md](./docs/api/protocol/rfc.md)

---

## Testing

### Running Tests

```bash
# Run all tests via CTest (4 test suites)
./r-type.sh test

# Run game unit tests directly
./r-type.sh tests

# Run specific test executable
./build/build/Release/bin/test_network
./build/build/Release/bin/test_game
```

### Test Coverage

**Active Test Suites:**
- `NetworkTests`: Protocol, serialization, UDP socket (82 tests)
- `ClientMinimalTest`: Basic client functionality
- `ClientUnitsTest`: Entity, SafeQueue, advanced client logic (40+ tests)
- `SanityTest`: Build verification

**Additional Test Executables (built but not in CTest):**
- `test_ecs`: Entity-Component-System tests
- `test_game`: Game logic (collision, movement, spawning, health, weapons, power-ups)
- `test_render`: Sprite and animation systems
- `test_integration`: Client-server integration tests

To enable additional tests, uncomment the corresponding `add_test()` calls in `tests/CMakeLists.txt`.

---

## Project Structure

```
R-TYPE/
├── client/             # Client application
│   ├── src/           # Game client, rendering, UI
│   └── include/       # Client headers
├── server/            # Server application
│   └── src/          # Game server, networking
├── engine/            # ECS engine core
│   └── ecs/          # Entity, component, registry
├── game-lib/          # Shared game logic
│   ├── entities/     # Enemy, projectile, boss factories
│   ├── systems/      # Game systems (collision, movement, etc.)
│   └── components/   # Game components
├── src/Common/        # Shared utilities
│   ├── Opcodes.hpp   # Network protocol definitions
│   └── BinarySerializer.hpp
├── tests/             # Test suites
├── assets/            # Game assets (sprites, sounds)
├── docs/              # Documentation
└── r-type.sh          # Build and run script
```

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



