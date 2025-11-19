# Scripts Documentation

This directory contains utility scripts for the R-TYPE project. Below is a comprehensive list of all available scripts and their usage.

---

## 📋 Table of Contents

- [Setup & Installation](#setup--installation)
  - [install-hooks.sh](#install-hookssh)
- [Code Quality](#code-quality)
  - [format.sh](#formatsh)

---

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

