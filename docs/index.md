# R-TYPE



---

##  About R-TYPE

R-TYPE is a networked multiplayer game remake of the classic side-scrolling shooter. Built with modern C++ and a custom Entity Component System (ECS), this project demonstrates advanced game development techniques including:

- **Custom ECS Architecture** - Efficient entity management and component-based design
- **Network Multiplayer** - UDP-based networking for real-time gameplay
- **Cross-platform** - Runs on Windows, Linux, and macOS
- **Modern C++17** - Clean, efficient, and maintainable codebase

##  Quick Start

```bash
# Clone the repository
git clone https://github.com/quent1111/R-TYPE.git
cd R-TYPE

# Install git hooks for code quality
./scripts/install-hooks.sh

# Build the project
mkdir build && cd build
cmake ..
make

# Run the server
./r-type_server

# Run the client (in another terminal)
./r-type_client
```

For detailed installation instructions, see [Installation Guide](getting-started/installation.md).

##  Documentation

- **[Getting Started](getting-started/installation.md)** - Install and run the game
- **[Architecture](architecture/overview.md)** - Learn about the project structure
- **[Developer Guide](developer-guide/contributing.md)** - Contribute to the project
- **[API Reference](api/client.md)** - Detailed API documentation

##  Features

- ✅ **Entity Component System (ECS)** - Flexible and performant game architecture
- ✅ **Network Multiplayer** - Play with friends over the network
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


