# R-TYPE

Welcome to the R-TYPE documentation!

---

##  About R-TYPE

R-TYPE is a modern reimplementation of the classic side-scrolling shooter game, featuring **4-player cooperative multiplayer**. Built with modern C++ and a custom Entity Component System (ECS), this project demonstrates advanced software architecture and game development techniques:

- **Custom ECS Architecture** - High-performance entity management with data-oriented design
- **Client-Server Multiplayer** - UDP-based networking with server authority
- **Refactored Architecture** - Clean separation of concerns (50-57% code reduction)
- **Modular Design** - State machines, managers, handlers, broadcasters
- **Cross-platform** - Runs on Windows, Linux, and macOS
- **Modern C++20** - Using concepts, ranges, and best practices
- **Production-Ready** - Comprehensive testing, CI/CD, documentation

##  Quick Start

```bash
# Clone the repository
git clone https://github.com/quent1111/R-TYPE.git
cd R-TYPE

# Build and run the server
./r-type.sh server

# Build and run the client
./r-type.sh client

# Or launch the admin panel
./r-type.sh admin
# Default admin password: admin123
```

**Controls:** Arrow keys to move, Space to shoot, ESC for menu

For detailed installation instructions, see [Installation Guide](getting-started/installation.md).

##  Documentation

- **[Getting Started](getting-started/quickstart.md)** - Quick start guide
- **[Building](getting-started/building.md)** - Detailed build instructions
- **[Architecture](architecture/overview.md)** - Learn about the project structure
- **[ECS System](architecture/ecs.md)** - Entity Component System deep dive
- **[Admin Panel](../admin-client/README.md)** - Server administration guide
- **[Developer Guide](developer-guide/contributing.md)** - Contribute to the project

##  Features

###  Implemented

#### Gameplay
- **4-Player Multiplayer** - Cooperative gameplay with server authority
- **Progressive Levels** - Increasing difficulty with wave-based enemies
- **Boss Fights** - Multi-phase boss encounters (Level 5+)
- **Power-up System** - Speed boost, shield, damage multiplier, drones (Support & Missile)
- **Weapon Upgrades** - Enhanced projectiles and special abilities
- **Score & Combo System** - Competitive scoring with multipliers
- **Health System** - Visual health bars and damage feedback
- **Friendly Fire** - Optional friendly fire mode with drone ally exclusion

#### User Experience
- **Keyboard Navigation** - Full arrow key navigation in menus and settings
- **Accessibility Features** - GLSL-based colorblind filters (Protanopia, Deuteranopia, Tritanopia, High Contrast)
- **Admin Panel** - Graphical server management interface with real-time monitoring
- **Settings Persistence** - Save and load user preferences

#### Architecture
- **Custom ECS Engine** - High-performance entity-component system
- **Client Architecture** - State machines (Menu/Lobby/Game), Singleton managers, Specialized renderers
- **Server Architecture** - Game session orchestration, Handler pattern, Broadcaster pattern
- **Network Layer** - Dual-loop UDP architecture with thread-safe queues
- **Audio System** - SFML-based sound effects and background music
- **Visual Effects** - Explosions, particles, screen shake, animations

#### Engineering
- **Modern C++20** - Using concepts, ranges, and best practices
- **Clean Architecture** - 50-57% code reduction through refactoring
- **Comprehensive Testing** - Unit, integration, and network tests
- **CI/CD Pipeline** - Automated builds, tests, and linting
- **Complete Documentation** - MkDocs with architecture diagrams
- **Cross-platform** - Windows, Linux, macOS support

##  Tech Stack

- **Language:** C++20 (concepts, ranges, coroutines)
- **Build System:** CMake 3.20+ with Conan 2.x
- **Graphics:** SFML 2.6.1 (window, rendering, audio)
- **Networking:** Asio 1.30.2 (async UDP)
- **Testing:** GTest 1.14.0
- **Architecture:** Custom ECS with data-oriented design
- **Documentation:** MkDocs with Material theme

##  Project Structure

```
R-TYPE/
├── client/         # Client application (503 lines, refactored)
│   ├── managers/   # Resource singletons (Texture, Audio, Effects)
│   ├── rendering/  # Specialized renderers (Game, HUD, Overlay)
│   ├── states/     # State machine (Menu, Lobby, Game)
│   └── network/    # NetworkClient with thread-safe queues
├── server/         # Server application (475 lines, refactored)
│   ├── game/       # Game logic (GameSession, Managers)
│   ├── handlers/   # Request processors (Input, Powerup, Weapon)
│   └── network/    # UDPServer, Broadcasters
├── engine/         # Custom game engine
│   ├── ecs/        # Entity Component System core
│   ├── audio/      # Audio subsystem
│   ├── render/     # Rendering utilities
│   ├── net/        # Network utilities
│   └── utils/      # Helper functions
├── game-lib/       # Shared game logic
│   ├── components/ # ECS components
│   ├── entities/   # Entity factories
│   └── systems/    # Game systems
├── tests/          # Comprehensive test suite
├── docs/           # Documentation source
└── assets/         # Game resources
```

## Contributing

We welcome contributions! Please read our [Contributing Guide](developer-guide/contributing.md) to get started.

## Development Setup

1. **Install dependencies**
   ```bash
   sudo apt install clang-format clang-tidy cmake build-essential
   ```

2. **Install git hooks**
   ```bash
   ./scripts/install-hooks.sh
   ```

3. **Follow our code style**
   - We use Google C++ Style Guide
   - Automatic formatting with clang-format
   - Static analysis with clang-tidy

See [Code Style Guide](developer-guide/code-style.md) for more details.

## License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/quent1111/R-TYPE/blob/main/LICENSE) file for details.


