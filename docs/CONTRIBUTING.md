# Contributing to R-Type

Thank you for your interest in contributing! This document provides guidelines for contributing to the R-Type project.

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help maintain a positive environment

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/G-CPP-500-NAN-5-2-rtype-4.git
   cd G-CPP-500-NAN-5-2-rtype-4
   ```
3. **Set up development environment**:
   ```bash
   chmod +x scripts/bootstrap.sh
   ./scripts/bootstrap.sh
   ```

## Development Workflow

### 1. Create a Feature Branch

```bash
git checkout -b feature/my-awesome-feature
```

Branch naming conventions:
- `feature/`: New features
- `fix/`: Bug fixes
- `refactor/`: Code refactoring
- `docs/`: Documentation updates
- `test/`: Test additions/modifications

### 2. Make Your Changes

- Follow the [Code Style](#code-style) guidelines
- Write tests for new functionality
- Update documentation as needed
- Keep commits atomic and well-described

### 3. Test Your Changes

```bash
# Build
cmake --build build

# Run tests
cd build && ctest --output-on-failure

# Check for warnings
cmake --build build 2>&1 | grep -i warning
```

### 4. Commit Your Changes

Write clear, descriptive commit messages:

```
Short summary (50 chars or less)

More detailed explanation if needed. Wrap at 72 characters.
Explain the problem this commit solves and why this approach
was chosen.

Fixes #123
```

### 5. Push and Create Pull Request

```bash
git push origin feature/my-awesome-feature
```

Then open a Pull Request on GitHub with:
- Clear title and description
- Reference to related issues
- Screenshots/GIFs for UI changes

## Code Style

### C++ Guidelines

- **Standard**: C++17
- **Style**: Google C++ Style Guide
- **Naming**:
  - Classes/Structs: `PascalCase`
  - Functions/Methods: `snake_case`
  - Variables: `snake_case`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: `_leading_underscore`

### Formatting

Use `clang-format` with provided `.clang-format`:

```bash
clang-format -i src/**/*.cpp include/**/*.hpp
```

### Best Practices

- **RAII**: Always use RAII for resource management
- **const correctness**: Mark const what can be const
- **Smart pointers**: Prefer `std::unique_ptr` and `std::shared_ptr` over raw pointers
- **Auto**: Use `auto` when type is obvious from context
- **Comments**: Write self-documenting code; comments should explain "why", not "what"

## Testing

### Writing Tests

Use GoogleTest framework:

```cpp
#include <gtest/gtest.h>
#include <ecs/entity.hpp>

TEST(EntityTest, ConstructorCreatesValidEntity) {
    entity_t entity(42);
    EXPECT_EQ(static_cast<size_t>(entity), 42);
}
```

### Test Coverage

- All public APIs must have tests
- Aim for >80% code coverage
- Test edge cases and error paths

## Documentation

### Code Documentation

Document public APIs with Doxygen-style comments:

```cpp
/**
 * @brief Spawns a new entity in the registry
 * @return The newly created entity ID
 */
entity_t spawn_entity();
```

### Architecture Documentation

Update `ARCHITECTURE.md` when:
- Adding new modules
- Changing system interactions
- Modifying design patterns

### Protocol Documentation

Update `docs/protocol.md` when:
- Adding new message types
- Changing packet format
- Modifying protocol behavior

## Pull Request Process

1. **Ensure CI passes**: GitHub Actions must be green
2. **Request review**: Tag at least one team member
3. **Address feedback**: Respond to all review comments
4. **Squash commits**: Clean up commit history before merge
5. **Merge**: Only maintainers can merge to `main`

## Issue Reporting

### Bug Reports

Use the bug report template and include:
- **Steps to reproduce**
- **Expected behavior**
- **Actual behavior**
- **Environment** (OS, compiler, versions)
- **Logs/Screenshots**

### Feature Requests

Use the feature request template and include:
- **Problem statement**: What problem does this solve?
- **Proposed solution**: How should it work?
- **Alternatives**: Other approaches considered?
- **Additional context**: Examples, mockups, etc.

## Questions?

- Open a GitHub Discussion for general questions
- Join our Discord (if available)
- Email the maintainers

---

**Happy coding! 🚀**
