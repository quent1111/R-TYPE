# R-TYPE

Welcome to the R-TYPE documentation!

---

##  About R-TYPE

R-TYPE is a modern reimplementation of the classic side-scrolling shooter, currently featuring a fully playable singleplayer demo. Built with modern C++ and a custom Entity Component System (ECS), this project demonstrates advanced game development techniques including:

- **Custom ECS Architecture** - Efficient entity management with data-oriented design
- **Sprite-Based Graphics** - SFML rendering with animated sprites and effects
- **Cross-platform** - Runs on Windows, Linux, and macOS  
- **Modern C++20** - Clean, efficient, and maintainable codebase
- **Multiplayer Ready** - Network layer planned for future UDP-based gameplay

##  Quick Start

```bash
# Clone the repository
git clone https://github.com/quent1111/R-TYPE.git
cd R-TYPE

# Build and run the client (auto-installs dependencies)
./r-type.sh client

# Or manually build
./r-type.sh build
cd build/build/Release/bin
./r-type_client
```

**Controls:** WASD to move, Space to shoot, ESC to exit

For detailed installation instructions, see [Installation Guide](getting-started/installation.md).

##  Documentation

- **[Getting Started](getting-started/quickstart.md)** - Quick start guide
- **[Building](getting-started/building.md)** - Detailed build instructions
- **[Architecture](architecture/overview.md)** - Learn about the project structure
- **[ECS System](architecture/ecs.md)** - Entity Component System deep dive
- **[Developer Guide](developer-guide/contributing.md)** - Contribute to the project

##  Features

### Current (Singleplayer Demo)
- ✅ **Entity Component System (ECS)** - Custom implementation with sparse arrays
- ✅ **Player Controls** - WASD movement with shooting mechanics
- ✅ **Enemy System** - Automatic wave spawning with AI movement
- ✅ **Collision Detection** - Projectile-enemy interactions
- ✅ **Visual Effects** - Explosion animations on enemy destruction
- ✅ **Sprite Animations** - Multi-frame animations for entities
- ✅ **Health System** - Player health with UI display
- ✅ **Scrolling Background** - Infinite parallax background

### Planned Features
- 🔜 **Network Multiplayer** - UDP-based client-server for 4 players
- 🔜 **Power-ups** - Weapon upgrades and shields
- 🔜 **Boss Enemies** - Multi-phase boss battles
- 🔜 **Audio System** - Sound effects and background music
- 🔜 **Score System** - Leaderboards and high scores
- ✅ **Custom Game Engine** - Built from scratch for maximum control
- ✅ **Modern Graphics** - Smooth rendering and visual effects
- ✅ **Cross-platform Support** - Windows, Linux, macOS

##  Tech Stack

- **Language:** C++17
- **Build System:** CMake
- **Graphics:** SFML / Custom Rendering
- **Networking:** UDP Sockets
- **Architecture:** ECS (Entity Component System)
- **Documentation:** MkDocs with Material theme

##  Project Structure

```
R-TYPE/
├── client/         # Client application
├── server/         # Server application
├── engine/         # Game engine core
├── bootstrap/      # ECS bootstrap implementation
├── docs/           # Documentation source files
├── scripts/        # Utility scripts
├── tests/          # Unit and integration tests
└── third_party/    # External dependencies
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


