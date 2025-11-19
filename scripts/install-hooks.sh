#!/bin/bash

# Install Git Hooks for R-TYPE
# This script installs pre-commit hooks for code quality checks

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
HOOKS_DIR="$PROJECT_ROOT/.git/hooks"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   Installing R-TYPE Git Hooks          ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}\n"

# Check if we're in a git repository
if [ ! -d "$PROJECT_ROOT/.git" ]; then
    echo -e "${RED}Error: Not in a git repository${NC}"
    exit 1
fi

# Create hooks directory if it doesn't exist
mkdir -p "$HOOKS_DIR"

# Pre-commit hook content
cat > "$HOOKS_DIR/pre-commit" << 'EOF'
#!/bin/bash

# R-TYPE Pre-commit Hook
# This hook runs before each commit to ensure code quality
# To bypass this hook in emergency: git commit --no-verify

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║      R-TYPE Pre-commit Checks          ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}\n"

# Get the list of staged files
STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM)
STAGED_CPP_FILES=$(echo "$STAGED_FILES" | grep -E '\.(cpp|hpp|h|cc|cxx)$' || true)

# Exit early if no C++ files are staged
if [ -z "$STAGED_CPP_FILES" ]; then
    echo -e "${GREEN}✓ No C++ files to check${NC}"
    exit 0
fi

# Count files
NUM_FILES=$(echo "$STAGED_CPP_FILES" | wc -l)
echo -e "${BLUE}Found $NUM_FILES C++ file(s) to check${NC}\n"

# Track if we had any failures
FAILURE=0

# ═══════════════════════════════════════════════════════════
# Check 1: Clang-format
# ═══════════════════════════════════════════════════════════
echo -e "${YELLOW}[1/3] Checking code formatting...${NC}"

if ! command -v clang-format &> /dev/null; then
    echo -e "${YELLOW}⚠ clang-format not found, skipping format check${NC}"
else
    FORMAT_ISSUES=0
    for file in $STAGED_CPP_FILES; do
        if [ -f "$file" ]; then
            # Check if file needs formatting
            if ! clang-format --dry-run -Werror "$file" &> /dev/null; then
                echo -e "${RED}  ✗ Needs formatting: $file${NC}"
                FORMAT_ISSUES=$((FORMAT_ISSUES + 1))
            fi
        fi
    done
    
    if [ $FORMAT_ISSUES -eq 0 ]; then
        echo -e "${GREEN}  ✓ All files are properly formatted${NC}\n"
    else
        echo -e "${RED}  ✗ $FORMAT_ISSUES file(s) need formatting${NC}"
        echo -e "${YELLOW}  → Run: ./scripts/format.sh -f${NC}\n"
        FAILURE=1
    fi
fi

# ═══════════════════════════════════════════════════════════
# Check 2: Clang-tidy (only for .cpp files)
# ═══════════════════════════════════════════════════════════
echo -e "${YELLOW}[2/3] Running static analysis...${NC}"

STAGED_CPP_SOURCE=$(echo "$STAGED_CPP_FILES" | grep -E '\.cpp$' || true)

if [ -z "$STAGED_CPP_SOURCE" ]; then
    echo -e "${GREEN}  ✓ No .cpp files to analyze${NC}\n"
elif ! command -v clang-tidy &> /dev/null; then
    echo -e "${YELLOW}⚠ clang-tidy not found, skipping analysis${NC}\n"
else
    TIDY_ISSUES=0
    for file in $STAGED_CPP_SOURCE; do
        if [ -f "$file" ]; then
            # Run clang-tidy, suppress output but capture return code
            if ! clang-tidy "$file" -- -std=c++17 &> /dev/null; then
                echo -e "${RED}  ✗ Issues found: $file${NC}"
                TIDY_ISSUES=$((TIDY_ISSUES + 1))
            fi
        fi
    done
    
    if [ $TIDY_ISSUES -eq 0 ]; then
        echo -e "${GREEN}  ✓ No static analysis issues${NC}\n"
    else
        echo -e "${RED}  ✗ $TIDY_ISSUES file(s) have issues${NC}"
        echo -e "${YELLOW}  → Run: ./scripts/format.sh -t${NC}"
        echo -e "${YELLOW}  → Or fix: ./scripts/format.sh -t -x${NC}\n"
        FAILURE=1
    fi
fi

# ═══════════════════════════════════════════════════════════
# Check 3: Forbidden patterns
# ═══════════════════════════════════════════════════════════
echo -e "${YELLOW}[3/3] Checking for forbidden patterns...${NC}"

FORBIDDEN_ISSUES=0

for file in $STAGED_CPP_FILES; do
    if [ -f "$file" ]; then
        # Check for debugging leftover
        if grep -qE "std::cout.*DEBUG|printf.*DEBUG|TODO.*FIXME" "$file"; then
            echo -e "${RED}  ✗ Debug code found: $file${NC}"
            FORBIDDEN_ISSUES=$((FORBIDDEN_ISSUES + 1))
        fi
        
        # Check for trailing whitespace
        if grep -qE " +$" "$file"; then
            echo -e "${RED}  ✗ Trailing whitespace: $file${NC}"
            FORBIDDEN_ISSUES=$((FORBIDDEN_ISSUES + 1))
        fi
    fi
done

if [ $FORBIDDEN_ISSUES -eq 0 ]; then
    echo -e "${GREEN}  ✓ No forbidden patterns found${NC}\n"
else
    echo -e "${RED}  ✗ $FORBIDDEN_ISSUES file(s) have forbidden patterns${NC}\n"
    FAILURE=1
fi

# ═══════════════════════════════════════════════════════════
# Final result
# ═══════════════════════════════════════════════════════════
echo -e "${BLUE}════════════════════════════════════════${NC}"

if [ $FAILURE -eq 0 ]; then
    echo -e "${GREEN}✓ All checks passed! Proceeding with commit.${NC}"
    exit 0
else
    echo -e "${RED}✗ Pre-commit checks failed!${NC}"
    echo -e "${YELLOW}Fix the issues above or use:${NC}"
    echo -e "${YELLOW}  git commit --no-verify${NC}"
    echo -e "${YELLOW}to bypass these checks (not recommended)${NC}"
    exit 1
fi
EOF

# Make hook executable
chmod +x "$HOOKS_DIR/pre-commit"

echo -e "${GREEN}✓ Pre-commit hook installed successfully!${NC}\n"

echo -e "${YELLOW}What happens now:${NC}"
echo -e "  • Before each commit, code will be automatically checked"
echo -e "  • Formatting (clang-format)"
echo -e "  • Static analysis (clang-tidy)"
echo -e "  • Forbidden patterns (debug code, trailing whitespace)"
echo -e ""
echo -e "${YELLOW}To bypass the hook in emergency:${NC}"
echo -e "  git commit --no-verify"
echo -e ""
echo -e "${BLUE}Happy coding! 🚀${NC}"
