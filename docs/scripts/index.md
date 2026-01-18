# Scripts Reference

Comprehensive reference for all utility scripts in the R-TYPE project.

## Main Build Script: `r-type.sh`

The all-in-one script for building, running, testing, and analyzing R-TYPE.

### Quick Start

```bash
# Build and run server
./r-type.sh server

# Build and run client
./r-type.sh client

# Run tests
./r-type.sh test
```

### Available Commands

| Command | Description |
|---------|-------------|
| `build` | Build the project (default: Release mode) |
| `client` | Build and run the game client |
| `server` | Build and run the game server |
| `admin` | Build and run the admin client |
| `test` | Build and run all 456 tests |
| `coverage` | Generate code coverage report (gcov/lcov) |
| `valgrind` | Run memory leak analysis with Valgrind |
| `clean` | Clean the build directory |
| `install` | Install Conan dependencies only |
| `rebuild` | Clean + rebuild from scratch |
| `all` | Build everything (client + server + tests) |

### Options

| Option | Short | Description |
|--------|-------|-------------|
| `--debug` | `-d` | Build in Debug mode (with symbols) |
| `--release` | `-r` | Build in Release mode (default, optimized) |
| `--clean` | `-c` | Clean before building |
| `--verbose` | `-v` | Show verbose build output |
| `--jobs N` | `-j N` | Number of parallel jobs (default: auto-detect) |
| `--help` | `-h` | Show help message |

### Usage Examples

#### Build Commands

```bash
# Simple release build
./r-type.sh build

# Debug build
./r-type.sh build --debug

# Clean rebuild with 8 parallel jobs
./r-type.sh rebuild -j 8

# Verbose debug build
./r-type.sh build --debug --verbose
```

#### Run Commands

```bash
# Run server (builds if needed)
./r-type.sh server

# Run client
./r-type.sh client

# Run admin panel
./r-type.sh admin

# Pass arguments to server
./r-type.sh server -- --port 5000 --verbose
```

#### Testing Commands

```bash
# Run all tests
./r-type.sh test

# Run tests in verbose mode
./r-type.sh test --verbose

# Run tests in debug mode
./r-type.sh test --debug
```

#### Analysis Commands

```bash
# Generate coverage report (Linux/macOS)
./r-type.sh coverage

# Memory leak check (Linux)
./r-type.sh valgrind

# Clean and rebuild everything
./r-type.sh all --clean
```

### Script Features

**Automatic Setup:**
- ✅ Installs Conan package manager if missing
- ✅ Downloads and builds dependencies (SFML, Asio, GTest, LZ4)
- ✅ Configures CMake with Conan toolchain
- ✅ Parallel builds (uses all CPU cores by default)

**Smart Build System:**
- Only rebuilds when necessary
- Caches Conan packages
- Detects build configuration changes
- Colored output for readability

**Cross-Platform:**
- Linux: Full support
- macOS: Full support
- Windows: Use `r-type.bat` (similar commands)

### Environment Variables

```bash
# Override build directory
BUILD_DIR=/custom/path ./r-type.sh build

# Force specific number of jobs
JOBS=4 ./r-type.sh build

# Use specific build type
BUILD_TYPE=Debug ./r-type.sh build
```

### Exit Codes

| Code | Meaning |
|------|---------|
| `0` | Success |
| `1` | Build/test failure |
| `2` | Dependency installation failed |
| `255` | Command error or invalid usage |

### Troubleshooting

#### Conan Not Found

```bash
# Script will auto-install, but if it fails:
pip install conan
export PATH="$HOME/.local/bin:$PATH"
```

#### Build Fails

```bash
# Clean everything and rebuild
./r-type.sh rebuild

# Check with verbose output
./r-type.sh build --verbose
```

#### Tests Fail

```bash
# Ensure release build
./r-type.sh test --release

# Run specific test
cd build/bin
./test_registry --gtest_filter="EntityTest.*"
```

---

## Documentation Scripts

### `scripts/docs.sh`

Manage MkDocs documentation.

```bash
# Install dependencies
./scripts/docs.sh install

# Start development server (with live reload)
./scripts/docs.sh serve

# Build static site
./scripts/docs.sh build

# Deploy to GitHub Pages
./scripts/docs.sh deploy
```

**Requirements:**
- Python 3.7+
- pip (Python package installer)

**Installed packages:**
- mkdocs
- mkdocs-material (theme)
- mkdocs-mermaid2-plugin (diagrams)

---

## Code Quality Scripts

### `scripts/format.sh`

Format and analyze code with clang-format and clang-tidy.

#### Format Code

```bash
# Format all source files
./scripts/format.sh -f

# Check formatting without changes
./scripts/format.sh -c
```

#### Lint/Analyze Code

```bash
# Run clang-tidy analysis
./scripts/format.sh -t

# Run all checks (format + tidy)
./scripts/format.sh -a
```

#### Options

| Option | Description |
|--------|-------------|
| `-f`, `--format` | Format code with clang-format |
| `-c`, `--check` | Check formatting (no changes) |
| `-t`, `--tidy` | Run clang-tidy analysis |
| `-a`, `--all` | Format + tidy |

**Affected Files:**
- `*.cpp`, `*.hpp`, `*.c`, `*.h` in:
  - `client/`
  - `server/`
  - `engine/`
  - `game-lib/`
  - `admin-client/`
  - `tests/`

**Configuration:**
- `.clang-format` - Formatting rules
- `.clang-tidy` - Linting rules

---

## Git Hooks

### `scripts/install-hooks.sh`

Install Git hooks for code quality.

```bash
./scripts/install-hooks.sh
```

**Installed Hooks:**
- `pre-commit` - Runs clang-format before commits
- Ensures code is formatted before committing

**Manual Hook Management:**

```bash
# Install hooks
./scripts/install-hooks.sh

# Remove hooks
rm .git/hooks/pre-commit
```

---

## CI/CD Scripts

### GitHub Actions Workflows

Located in `.github/workflows/`:

| Workflow | Trigger | Description |
|----------|---------|-------------|
| `ci.yml` | Push, PR | Build + test on Linux |
| `pr-check.yml` | PR | Quality checks for PRs |
| `coverage.yml` | Push, PR | Generate coverage reports |

**Note:** Workflows currently use `ubuntu-latest` runners (GitHub-hosted).

---

## Advanced Usage

### Combining Commands

```bash
# Clean, build in debug, run tests
./r-type.sh rebuild --debug && ./r-type.sh test --debug

# Build and generate coverage
./r-type.sh build --debug && ./r-type.sh coverage

# Format code and build
./scripts/format.sh -f && ./r-type.sh build
```

### Custom Build Configurations

```bash
# Build with specific CMake options
cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build . -j$(nproc)
```

### Parallel Workflow

```bash
# Terminal 1: Run server
./r-type.sh server

# Terminal 2: Run client
./r-type.sh client

# Terminal 3: Run tests in watch mode
watch -n 2 './r-type.sh test'
```

---

## Script Internals

### Build Process

1. **Check Conan**: Install if missing
2. **Install Dependencies**: `conan install` with SFML, Asio, GTest, LZ4
3. **Configure CMake**: Use Conan toolchain
4. **Build**: Parallel compilation with all cores
5. **Run**: Execute binaries from `build/bin/`

### Directory Structure

```
R-TYPE/
├── r-type.sh              # Main script (Linux/macOS)
├── r-type.bat             # Windows equivalent
├── scripts/
│   ├── docs.sh            # Documentation management
│   ├── format.sh          # Code formatting/linting
│   └── install-hooks.sh   # Git hooks installer
├── build/                 # Build output (auto-created)
│   ├── bin/               # Compiled binaries
│   ├── conan_toolchain.cmake
│   └── ...
└── .github/workflows/     # CI/CD configuration
```

---

## Quick Reference Card

### Most Common Commands

```bash
# Build everything
./r-type.sh build

# Run game
./r-type.sh server  # Terminal 1
./r-type.sh client  # Terminal 2

# Development cycle
./scripts/format.sh -f      # Format
./r-type.sh build --debug   # Build
./r-type.sh test            # Test

# Analysis
./r-type.sh coverage        # Coverage
./r-type.sh valgrind        # Memory leaks

# Documentation
./scripts/docs.sh serve     # Live preview
./scripts/docs.sh build     # Static site
```

---

## See Also

- [Installation Guide](../getting-started/installation.md) - Setup instructions
- [Building Guide](../getting-started/building.md) - Detailed build info
- [Testing Guide](../developer-guide/testing.md) - Test suite documentation
- [Contributing Guide](../developer-guide/contributing.md) - Development workflow
