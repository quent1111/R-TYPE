# Code Style Guide

R-TYPE follows the **Google C++ Style Guide** with automatic enforcement.

## Formatting

Code is automatically formatted with **clang-format**:

```bash
# Format your code
./scripts/format.sh -f
```

## Static Analysis

Code is analyzed with **clang-tidy**:

```bash
# Run analysis
./scripts/format.sh -t

# Auto-fix issues
./scripts/format.sh -t -x
```

## Naming Conventions

- **Classes/Structs**: `CamelCase`
- **Functions**: `camelBack`
- **Variables**: `lower_case`
- **Private members**: `member_name_`
- **Constants**: `kConstantName`
- **Macros**: `MACRO_NAME`

## File Organization

```cpp
// 1. Header comment
// 2. Include guard
// 3. Includes (project, then system)
// 4. Forward declarations
// 5. Class/function declarations
// 6. Inline functions
```

## Examples

See existing code for examples of proper style.
