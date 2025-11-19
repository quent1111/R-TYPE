# Scripts Documentation

This directory contains utility scripts for the R-TYPE project. Below is a comprehensive list of all available scripts and their usage.

---

## 📋 Table of Contents

- [Build & Run](#build--run)
  - [r-type.sh / r-type.bat](#r-typesh--r-typebat) ⭐ **Main Script**
- [Setup & Installation](#setup--installation)
  - [install-hooks.sh](#install-hookssh)
- [Documentation](#documentation)
  - [docs.sh](#docssh)
- [Code Quality](#code-quality)
  - [format.sh](#formatsh)

---

## Build & Run

### `r-type.sh` / `r-type.bat`

**⭐ Main project script for simplified build, run, and test workflows.**

**Location:** `r-type.sh` (Linux/macOS) or `r-type.bat` (Windows)

**Requirements:**
- Python 3 + pip (for Conan)
- CMake 3.20+
- C++20 compiler (GCC 10+, Clang 11+, MSVC 2019+)

**Note:** Conan is **automatically installed** if not found!

#### What it does

This unified script handles the entire project lifecycle:
- 🔧 **Automatic dependency installation** via Conan
- 🏗️ **CMake configuration** with proper toolchain
- ⚡ **Parallel building** with optimized CPU usage
- 🎮 **Run server or client** with one command
- 🧪 **Testing** with CTest
- 📊 **Code coverage** reports (Linux/macOS)
- 🔍 **Memory leak detection** with Valgrind (Linux)

#### Commands

| Command | Description |
|---------|-------------|
| `build` | Build the entire project |
| `client` | Build and run the client |
| `server` | Build and run the server |
| `test` | Run all tests with CTest |
| `coverage` | Generate code coverage report (requires lcov) |
| `valgrind` | Run server with Valgrind memory check |
| `clean` | Clean build directory |
| `install` | Install dependencies only (no build) |
| `rebuild` | Clean and rebuild from scratch |
| `all` | Build + run tests + generate coverage |

#### Options

| Option | Description |
|--------|-------------|
| `--debug` | Build in Debug mode (with symbols) |
| `--release` | Build in Release mode (optimized, default) |
| `--clean` | Clean before building |
| `--verbose` | Show verbose build output |
| `--jobs N` / `-j N` | Number of parallel jobs (default: CPU cores) |
| `--help` / `-h` | Show help message |

#### Examples

**Quick Start:**
```bash
# Linux/macOS - Run server immediately
./r-type.sh server

# Windows - Run server immediately
r-type.bat server
```

**Build Commands:**
```bash
# Build everything (Release mode)
./r-type.sh build

# Build in Debug mode
./r-type.sh build --debug

# Clean build from scratch
./r-type.sh rebuild

# Build with 8 parallel jobs
./r-type.sh build -j 8

# Verbose Debug build
./r-type.sh build --debug --verbose
```

**Run Commands:**
```bash
# Run client
./r-type.sh client

# Run server
./r-type.sh server

# Run client in Debug mode
./r-type.sh client --debug
```

**Testing & Quality:**
```bash
# Run all tests
./r-type.sh test

# Generate code coverage report
./r-type.sh coverage

# Run server with Valgrind (memory leak detection)
./r-type.sh valgrind

# Full quality check (build + test + coverage)
./r-type.sh all
```

**Maintenance:**
```bash
# Install/update dependencies only
./r-type.sh install

# Clean build artifacts
./r-type.sh clean

# Full rebuild
./r-type.sh rebuild
```

#### For New Contributors

**First time setup:**
```bash
# 1. Clone repository
git clone https://github.com/quent1111/R-TYPE.git
cd R-TYPE

# 2. Install Git hooks (code quality)
./scripts/install-hooks.sh

# 3. Run server (everything auto-installs!)
./r-type.sh server
```

That's it! The script handles all dependencies automatically.


## Setup & Installation

### `install-hooks.sh`

**Purpose:** Install Git hooks (pre-commit) for automatic code quality checks.

**Location:** `scripts/install-hooks.sh`

**Requirements:**
- Git repository
- `clang-format` (optional, for formatting checks)
- `clang-tidy` (optional, for static analysis)

#### What it does

Installs a **pre-commit hook** that automatically runs before each commit to:
1. ✅ Check code formatting (clang-format)
2. ✅ Run static analysis (clang-tidy)
3. ✅ Detect forbidden patterns (debug code, trailing whitespace)

If any check fails, the commit is blocked until issues are fixed.

#### Usage

```bash
# Install the hooks (run once after cloning)
./scripts/install-hooks.sh
```

#### Pre-commit Hook Features

**Automatic checks on staged C++ files:**
- Code formatting validation
- Static analysis with clang-tidy
- Forbidden patterns detection:
  - Debug leftover code (`DEBUG` in cout/printf)
  - Trailing whitespace
  - TODO/FIXME comments

**Bypass the hook (emergency only):**
```bash
git commit --no-verify
```

#### For Team Members

After cloning the repository, run:
```bash
./scripts/install-hooks.sh
```

This ensures everyone has the same quality standards enforced automatically.

---

## Documentation

### `docs.sh`

**Purpose:** Build and serve project documentation using MkDocs with Material theme.

**Location:** `scripts/docs.sh`

**Requirements:**
- Python 3.8+
- pip3

#### Commands

| Command | Description |
|---------|-------------|
| `serve` | Start development server (default) |
| `build` | Build static site |
| `deploy` | Deploy to GitHub Pages |
| `install` | Install MkDocs dependencies |
| `clean` | Remove built documentation |

#### Options

| Option | Description |
|--------|-------------|
| `-p, --port PORT` | Port for dev server (default: 8000) |
| `-a, --addr ADDR` | Address to bind (default: 127.0.0.1) |
| `-h, --help` | Show help message |

#### Examples

```bash
# First time setup - install dependencies
./scripts/docs.sh install

# Start development server (live reload)
./scripts/docs.sh serve

# Serve on custom port
./scripts/docs.sh serve -p 9000

# Build static site
./scripts/docs.sh build

# Deploy to GitHub Pages
./scripts/docs.sh deploy

# Clean built files
./scripts/docs.sh clean
```

#### Documentation Structure

The documentation source is in `docs/`:
- `index.md` - Home page
- `getting-started/` - Installation and quickstart
- `architecture/` - System architecture
- `developer-guide/` - Contributing and coding standards
- `api/` - API reference

#### Editing Documentation

1. Edit markdown files in `docs/`
2. Run `./scripts/docs.sh serve` to preview changes
3. Documentation auto-reloads on file changes
4. Visit http://localhost:8000

#### Theme and Features

The documentation uses **Material for MkDocs** with:
- ✅ Dark/Light mode toggle
- ✅ Search functionality
- ✅ Code syntax highlighting
- ✅ Mobile-responsive design
- ✅ Git revision dates
- ✅ Navigation tabs

---

## Code Quality

### `format.sh`

**Purpose:** Format and analyze C++ code using clang-format and clang-tidy.

**Location:** `scripts/format.sh`

**Requirements:**
- `clang-format` (install: `sudo apt install clang-format`)
- `clang-tidy` (install: `sudo apt install clang-tidy`)

#### Usage

```bash
./scripts/format.sh [OPTIONS]
```

#### Options

| Option | Long Form | Description |
|--------|-----------|-------------|
| `-f` | `--format` | Format code with clang-format |
| `-t` | `--tidy` | Run clang-tidy analysis |
| `-a` | `--all` | Run both format and tidy |
| `-x` | `--fix` | Auto-fix issues (for tidy) |
| `-d DIR` | `--dir DIR` | Specify directory to process (can be used multiple times) |
| `-h` | `--help` | Show help message |

#### Examples

```bash
# Check formatting only (default)
./scripts/format.sh

# Format all code
./scripts/format.sh -f

# Run clang-tidy analysis
./scripts/format.sh -t

# Format and analyze everything
./scripts/format.sh -a

# Format, analyze and auto-fix issues
./scripts/format.sh -a -x

# Format only the client directory
./scripts/format.sh -f -d client

# Format only the server directory
./scripts/format.sh -f -d server
```

