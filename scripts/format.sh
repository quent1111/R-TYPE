#!/bin/bash

# R-TYPE Code Formatter and Linter Script
# This script formats and analyzes C++ code using clang-format and clang-tidy

# NOTE: We don't use 'set -e' to allow checking all files even if some fail
set -u  # Error on undefined variables

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

MODE="check"
FIX=false
TIDY=false
DIRECTORIES=("client" "server" "engine" "bootstrap")

show_help() {
    cat << EOF
Usage: $0 [OPTIONS]

Format and analyze C++ code in the R-TYPE project.

OPTIONS:
    -f, --format        Format code with clang-format (default: check only)
    -t, --tidy          Run clang-tidy analysis
    -a, --all           Run both format and tidy
    -x, --fix           Auto-fix issues (for tidy)
    -d, --dir DIR       Specify directory to process (can be used multiple times)
    -h, --help          Show this help message

EXAMPLES:
    $0                  # Check formatting only
    $0 -f               # Format all code
    $0 -t               # Run tidy analysis
    $0 -a -x            # Format and auto-fix with tidy
    $0 -f -d client     # Format only the client directory

EOF
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -f|--format)
            MODE="format"
            shift
            ;;
        -t|--tidy)
            TIDY=true
            shift
            ;;
        -a|--all)
            MODE="format"
            TIDY=true
            shift
            ;;
        -x|--fix)
            FIX=true
            shift
            ;;
        -d|--dir)
            DIRECTORIES=("$2")
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

check_clang_format() {
    if ! command -v clang-format &> /dev/null; then
        echo -e "${RED}Error: clang-format is not installed${NC}"
        echo "Install it with: sudo apt install clang-format"
        exit 1
    fi
}

check_clang_tidy() {
    if ! command -v clang-tidy &> /dev/null; then
        echo -e "${RED}Error: clang-tidy is not installed${NC}"
        echo "Install it with: sudo apt install clang-tidy"
        exit 1
    fi
}

find_cpp_files() {
    local files=()
    for dir in "${DIRECTORIES[@]}"; do
        if [[ -d "$PROJECT_ROOT/$dir" ]]; then
            while IFS= read -r -d '' file; do
                files+=("$file")
            done < <(find "$PROJECT_ROOT/$dir" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" -o -name "*.cxx" \) -print0)
        fi
    done
    echo "${files[@]}"
}

format_code() {
    check_clang_format
    
    local files=($(find_cpp_files))
    
    if [[ ${#files[@]} -eq 0 ]]; then
        echo -e "${YELLOW}No C++ files found in specified directories${NC}"
        return 0
    fi
    
    echo -e "${BLUE}=== Formatting C++ files ===${NC}"
    echo -e "Found ${#files[@]} files\n"
    
    local formatted=0
    for file in "${files[@]}"; do
        if [[ "$MODE" == "format" ]]; then
            echo -e "${GREEN}Formatting:${NC} $file"
            clang-format -i "$file" || echo -e "${RED}Failed to format: $file${NC}"
            ((formatted++))
        else
            if ! clang-format --dry-run -Werror "$file" &> /dev/null; then
                echo -e "${YELLOW}Needs formatting:${NC} $file"
                ((formatted++))
            fi
        fi
    done
    
    if [[ "$MODE" == "format" ]]; then
        echo -e "\n${GREEN}✓ Formatted $formatted files${NC}"
        return 0
    else
        if [[ $formatted -eq 0 ]]; then
            echo -e "\n${GREEN}✓ All files are properly formatted${NC}"
            return 0
        else
            echo -e "\n${YELLOW}⚠ $formatted files need formatting${NC}"
            echo -e "Run with ${BLUE}-f${NC} to format them"
            return 1
        fi
    fi
}

run_tidy() {
    check_clang_tidy
    
    local files=($(find_cpp_files))
    
    if [[ ${#files[@]} -eq 0 ]]; then
        echo -e "${YELLOW}No C++ files found in specified directories${NC}"
        return 0
    fi
    
    echo -e "${BLUE}=== Running clang-tidy analysis ===${NC}"
    echo -e "Analyzing ${#files[@]} files\n"
    
    local issues=0
    for file in "${files[@]}"; do
        if [[ "$file" == *.cpp ]]; then
            echo -e "${BLUE}Analyzing:${NC} $file"
            
            if [[ "$FIX" == true ]]; then
                if ! clang-tidy -fix "$file" -- -std=c++17 2>/dev/null; then
                    ((issues++))
                fi
            else
                if ! clang-tidy "$file" -- -std=c++17 2>/dev/null; then
                    ((issues++))
                fi
            fi
        fi
    done
    
    if [[ $issues -eq 0 ]]; then
        echo -e "\n${GREEN}✓ No issues found${NC}"
        return 0
    else
        echo -e "\n${YELLOW}⚠ Found issues in $issues files${NC}"
        if [[ "$FIX" == false ]]; then
            echo -e "Run with ${BLUE}-x${NC} to auto-fix some issues"
        fi
        return 1
    fi
}

cd "$PROJECT_ROOT"

echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║   R-TYPE Code Formatter & Analyzer     ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}\n"

FORMAT_EXIT=0
TIDY_EXIT=0

if [[ "$MODE" == "format" ]] || [[ "$TIDY" == false ]]; then
    format_code
    FORMAT_EXIT=$?
    echo ""
fi

if [[ "$TIDY" == true ]]; then
    run_tidy
    TIDY_EXIT=$?
fi

echo -e "\n${GREEN}Done!${NC}"

# Exit with error if any check failed
if [[ $FORMAT_EXIT -ne 0 ]] || [[ $TIDY_EXIT -ne 0 ]]; then
    exit 1
fi
