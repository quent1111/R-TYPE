# Scripts Documentation

This directory contains utility scripts for the R-TYPE project. Below is a comprehensive list of all available scripts and their usage.

---

## 📋 Table of Contents

- [Code Quality](#code-quality)
  - [format.sh](#formatsh)

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

