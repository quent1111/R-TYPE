#!/bin/bash
# Pre-push validation script
# Run this before pushing to ensure CI will pass

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔═══════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║                                                       ║${NC}"
echo -e "${BLUE}║              R-TYPE Pre-Push Validation               ║${NC}"
echo -e "${BLUE}║                                                       ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════╝${NC}"
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

ERRORS=0

# 1. Check formatting
echo -e "${YELLOW}▶${NC} Checking code formatting..."
if ./scripts/format.sh > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Code formatting is correct"
else
    echo -e "${RED}✗${NC} Code formatting issues found"
    echo -e "${YELLOW}  Run: ./scripts/format.sh --format${NC}"
    ERRORS=$((ERRORS + 1))
fi

# 2. Check if project builds
echo -e "${YELLOW}▶${NC} Checking if project builds..."
if ./r-type.sh build > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Project builds successfully"
else
    echo -e "${RED}✗${NC} Build failed"
    echo -e "${YELLOW}  Run: ./r-type.sh build --verbose${NC}"
    ERRORS=$((ERRORS + 1))
fi

# 3. Run tests
echo -e "${YELLOW}▶${NC} Running tests..."
if ./r-type.sh test > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} All tests pass"
else
    echo -e "${RED}✗${NC} Some tests failed"
    echo -e "${YELLOW}  Run: ./r-type.sh test${NC}"
    ERRORS=$((ERRORS + 1))
fi

echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✅ All checks passed! Safe to push.${NC}"
    echo ""
    exit 0
else
    echo -e "${RED}❌ $ERRORS check(s) failed!${NC}"
    echo ""
    echo -e "${YELLOW}Fix the issues above before pushing.${NC}"
    echo ""
    exit 1
fi
