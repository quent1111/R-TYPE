#!/bin/bash
# CI/CD Setup Verification Script
# Checks that all required files are in place

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}╔═══════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║                                                       ║${NC}"
echo -e "${BLUE}║           CI/CD Setup Verification                    ║${NC}"
echo -e "${BLUE}║                                                       ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════╝${NC}"
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

MISSING=0

check_file() {
    local file=$1
    local desc=$2
    
    if [ -f "$file" ]; then
        echo -e "${GREEN}✓${NC} $desc"
        return 0
    else
        echo -e "${RED}✗${NC} $desc (missing: $file)"
        MISSING=$((MISSING + 1))
        return 1
    fi
}

echo -e "${YELLOW}Checking GitHub Actions Workflows...${NC}"
check_file ".github/workflows/ci.yml" "Main CI/CD Pipeline"
check_file ".github/workflows/pr-check.yml" "Pull Request Checks"
check_file ".github/workflows/coverage.yml" "Code Coverage"
echo ""

echo -e "${YELLOW}Checking Documentation...${NC}"
check_file ".github/CI_SETUP.md" "Setup Guide"
check_file ".github/CI_COMPLETE.md" "Complete Documentation"
check_file ".github/README.md" "Quick Reference"
check_file "CI_SUMMARY.md" "Summary (root)"
check_file "CI_QUICK_START.md" "Quick Start (root)"
echo ""

echo -e "${YELLOW}Checking Scripts...${NC}"
check_file "scripts/format.sh" "Format Script (existing)"
check_file "scripts/pre-push.sh" "Pre-push Validation"
check_file "r-type.sh" "Build Script"
echo ""

echo -e "${YELLOW}Checking Configuration Files...${NC}"
check_file ".clang-format" "Clang-format config"
check_file ".clang-tidy" "Clang-tidy config"
check_file "conanfile.txt" "Conan dependencies"
check_file "CMakeLists.txt" "CMake configuration"
echo ""

# Check if scripts are executable
echo -e "${YELLOW}Checking Script Permissions...${NC}"
if [ -x "scripts/format.sh" ]; then
    echo -e "${GREEN}✓${NC} format.sh is executable"
else
    echo -e "${RED}✗${NC} format.sh is not executable"
    echo -e "${YELLOW}  Run: chmod +x scripts/format.sh${NC}"
    MISSING=$((MISSING + 1))
fi

if [ -x "scripts/pre-push.sh" ]; then
    echo -e "${GREEN}✓${NC} pre-push.sh is executable"
else
    echo -e "${RED}✗${NC} pre-push.sh is not executable"
    echo -e "${YELLOW}  Run: chmod +x scripts/pre-push.sh${NC}"
    MISSING=$((MISSING + 1))
fi

if [ -x "r-type.sh" ]; then
    echo -e "${GREEN}✓${NC} r-type.sh is executable"
else
    echo -e "${RED}✗${NC} r-type.sh is not executable"
    echo -e "${YELLOW}  Run: chmod +x r-type.sh${NC}"
    MISSING=$((MISSING + 1))
fi
echo ""

# Check GitHub token (if possible)
echo -e "${YELLOW}Checking GitHub Configuration...${NC}"
if gh secret list 2>/dev/null | grep -q "EPITECH_DEPLOY_TOKEN"; then
    echo -e "${GREEN}✓${NC} EPITECH_DEPLOY_TOKEN secret is configured"
else
    echo -e "${YELLOW}⚠${NC} EPITECH_DEPLOY_TOKEN secret not found (optional for now)"
    echo -e "${YELLOW}  Setup: See .github/CI_SETUP.md${NC}"
fi
echo ""

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

if [ $MISSING -eq 0 ]; then
    echo -e "${GREEN}✅ CI/CD setup is complete!${NC}"
    echo ""
    echo -e "${BLUE}Next steps:${NC}"
    echo -e "  1. Configure EPITECH_DEPLOY_TOKEN (see .github/CI_SETUP.md)"
    echo -e "  2. Create a test PR to verify workflows"
    echo -e "  3. Merge to main to trigger deployment"
    echo ""
    exit 0
else
    echo -e "${RED}❌ $MISSING file(s) missing or not executable!${NC}"
    echo ""
    echo -e "${YELLOW}Please fix the issues above.${NC}"
    echo ""
    exit 1
fi
