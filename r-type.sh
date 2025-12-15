#!/bin/bash

# R-TYPE - All-in-One Script
# Build, run, test, and analyze the R-TYPE project
# Usage: ./r-type.sh [command] [options]

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

get_bin_dir() {
    if [ -d "$BUILD_DIR/$BUILD_TYPE/bin" ]; then
        echo "$BUILD_DIR/$BUILD_TYPE/bin"
    elif [ -d "$BUILD_DIR/build/$BUILD_TYPE/bin" ]; then
        echo "$BUILD_DIR/build/$BUILD_TYPE/bin"
    elif [ -d "$BUILD_DIR/bin" ]; then
        echo "$BUILD_DIR/bin"
    else
        echo "$BUILD_DIR/bin"
    fi
}

BUILD_TYPE="Release"
CLEAN_BUILD=false
VERBOSE=false
JOBS=$(nproc 2>/dev/null || echo 4)
COMMAND=""

print_banner() {
    echo -e "${CYAN}"
    echo "╔═══════════════════════════════════════════════════════╗"
    echo "║                                                       ║"
    echo "║                   R-TYPE Manager                      ║"
    echo "║            Build • Run • Test • Analyze               ║"
    echo "║                                                       ║"
    echo "╚═══════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

show_help() {
    print_banner
    cat << EOF
${YELLOW}USAGE:${NC}
    ./r-type.sh [COMMAND] [OPTIONS]

${YELLOW}COMMANDS:${NC}
    ${GREEN}build${NC}               Build the project (default: Release)
    ${GREEN}client${NC}              Build and run the client
    ${GREEN}server${NC}              Build and run the server
    ${GREEN}test${NC}                Build and run tests
    ${GREEN}tests${NC}               Build and run game unit tests only
    ${GREEN}coverage${NC}            Generate code coverage report
    ${GREEN}valgrind${NC}            Run memory leak analysis
    ${GREEN}clean${NC}               Clean build directory
    ${GREEN}install${NC}             Install dependencies only
    ${GREEN}rebuild${NC}             Clean and build from scratch
    ${GREEN}all${NC}                 Build everything (client + server + tests)

${YELLOW}OPTIONS:${NC}
    -d, --debug             Build in Debug mode
    -r, --release           Build in Release mode (default)
    -c, --clean             Clean before building
    -v, --verbose           Verbose output
    -j, --jobs N            Number of parallel jobs (default: auto)
    -h, --help              Show this help message

${YELLOW}EXAMPLES:${NC}
    ${CYAN}# Quick start (build + run server)${NC}
    ./r-type.sh server

    ${CYAN}# Build in debug mode${NC}
    ./r-type.sh build --debug

    ${CYAN}# Clean rebuild in release${NC}
    ./r-type.sh rebuild

    ${CYAN}# Run tests${NC}
    ./r-type.sh test

    ${CYAN}# Run game unit tests only${NC}
    ./r-type.sh tests

    ${CYAN}# Run client${NC}
    ./r-type.sh client

    ${CYAN}# Generate coverage report${NC}
    ./r-type.sh coverage

    ${CYAN}# Check for memory leaks${NC}
    ./r-type.sh valgrind

${YELLOW}DEPENDENCIES:${NC}
    The script will automatically:
    • Install Conan if needed
    • Download and build SFML, Asio, GTest
    • Configure CMake with proper toolchain
    • No manual installation required!

EOF
}

command_exists() {
    command -v "$1" &> /dev/null
}

print_step() {
    echo -e "\n${BLUE}▶ $1${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

check_conan() {
    print_step "Checking Conan installation..."
    if ! command_exists conan; then
        print_warning "Conan not found. Installing..."
        if ! command_exists pip3 && ! command_exists pip; then
            print_error "pip not found. Please install Python 3 and pip first."
            echo "  Ubuntu/Debian: sudo apt install python3-pip"
            echo "  macOS: brew install python3"
            exit 1
        fi
        local PIP_CMD=$(command_exists pip3 && echo "pip3" || echo "pip")
        $PIP_CMD install --user conan
        if ! command_exists conan; then
            export PATH="$HOME/.local/bin:$PATH"
        fi
        conan profile detect --force
        print_success "Conan installed successfully"
    else
        print_success "Conan is already installed"
    fi
}

check_dev_tools() {
    local INSTALL_DEV_TOOLS=false
    for arg in "$@"; do
        if [[ "$arg" == "coverage" ]] || [[ "$arg" == "valgrind" ]] || [[ "$arg" == "all" ]]; then
            INSTALL_DEV_TOOLS=true
            break
        fi
    done
    if [ "$INSTALL_DEV_TOOLS" = false ]; then
        return 0
    fi
    print_step "Checking development tools..."
    local MISSING_TOOLS=()
    if ! command_exists lcov; then
        MISSING_TOOLS+=("lcov")
    fi
    if ! command_exists valgrind; then
        MISSING_TOOLS+=("valgrind")
    fi
    if [ ${#MISSING_TOOLS[@]} -eq 0 ]; then
        print_success "All development tools are installed"
        return 0
    fi
    if command_exists dnf; then
        print_warning "Missing tools: ${MISSING_TOOLS[*]}"
        echo "Installing with dnf..."
        sudo dnf install -y "${MISSING_TOOLS[@]}"
    elif command_exists apt-get; then
        print_warning "Missing tools: ${MISSING_TOOLS[*]}"
        echo "Installing with apt..."
        sudo apt-get update && sudo apt-get install -y "${MISSING_TOOLS[@]}"
    elif command_exists brew; then
        print_warning "Missing tools: ${MISSING_TOOLS[*]}"
        echo "Installing with Homebrew..."
        brew install "${MISSING_TOOLS[@]}"
    else
        print_error "Could not determine package manager. Please install manually:"
        echo "  ${MISSING_TOOLS[*]}"
        exit 1
    fi
    print_success "Development tools installed"
}

check_cmake() {
    print_step "Checking CMake..."
    if ! command_exists cmake; then
        print_error "CMake not found. Please install CMake 3.20 or higher."
        echo "  Ubuntu/Debian: sudo apt install cmake"
        echo "  macOS: brew install cmake"
        exit 1
    fi
    CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
    print_success "CMake $CMAKE_VERSION found"
}

check_compiler() {
    print_step "Checking C++ compiler..."
    if command_exists g++; then
        GCC_VERSION=$(g++ --version | head -n1 | awk '{print $NF}')
        print_success "GCC $GCC_VERSION found"
    elif command_exists clang++; then
        CLANG_VERSION=$(clang++ --version | head -n1 | awk '{print $NF}')
        print_success "Clang $CLANG_VERSION found"
    else
        print_error "No C++ compiler found. Please install g++ or clang++."
        exit 1
    fi
}

install_dependencies() {
    cd "$PROJECT_ROOT"
    if [ -f "$BUILD_DIR/conan_toolchain.cmake" ] || \
       [ -f "$BUILD_DIR/$BUILD_TYPE/generators/conan_toolchain.cmake" ] || \
       [ -f "$BUILD_DIR/build/$BUILD_TYPE/generators/conan_toolchain.cmake" ] || \
       [ -f "$BUILD_DIR/generators/conan_toolchain.cmake" ]; then
        print_success "Dependencies already installed (skipping)"
        return 0
    fi
    print_step "Installing dependencies with Conan (this may take a few minutes)..."
    mkdir -p "$BUILD_DIR"
    local BUILD_TYPE_LOWER=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
    local CONAN_ARGS=(
        "--output-folder=$BUILD_DIR"
        "--build=missing"
        "-s" "build_type=$BUILD_TYPE"
        "-c" "tools.system.package_manager:mode=install"
        "-c" "tools.system.package_manager:sudo=True"
    )
    if [[ "$OSTYPE" == "darwin"* ]] && [[ "$(uname -m)" == "arm64" ]]; then
        CONAN_ARGS+=("--build=freetype/*")
    fi
    if [ "$VERBOSE" = true ]; then
        conan install . "${CONAN_ARGS[@]}"
    else
        conan install . "${CONAN_ARGS[@]}" > "$BUILD_DIR/conan-install.log" 2>&1
        if [ $? -ne 0 ]; then
            print_error "Conan installation failed. Check $BUILD_DIR/conan-install.log for details."
            tail -n 20 "$BUILD_DIR/conan-install.log"
            exit 1
        fi
    fi
    print_success "Dependencies installed"
}

configure_cmake() {
    print_step "Configuring CMake..."
    cd "$PROJECT_ROOT"
    if [ -f "CMakeUserPresets.json" ]; then
        if [ "$VERBOSE" = true ]; then
            cmake --preset conan-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
            local CMAKE_EXIT=$?
        else
            cmake --preset conan-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]') > "$BUILD_DIR/cmake-config.log" 2>&1
            local CMAKE_EXIT=$?
        fi
        if [ $CMAKE_EXIT -ne 0 ]; then
            print_error "CMake configuration failed"
            echo ""
            if [ "$VERBOSE" = false ]; then
                echo "Last 30 lines of $BUILD_DIR/cmake-config.log:"
                echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
                tail -n 30 "$BUILD_DIR/cmake-config.log"
                echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
                echo ""
                echo "Tip: Run with --verbose for full output"
            fi
            exit 1
        fi
    else
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        local TOOLCHAIN_FILE=""
        if [ -f "conan_toolchain.cmake" ]; then
            TOOLCHAIN_FILE="conan_toolchain.cmake"
        elif [ -f "build/$BUILD_TYPE/generators/conan_toolchain.cmake" ]; then
            TOOLCHAIN_FILE="build/$BUILD_TYPE/generators/conan_toolchain.cmake"
        elif [ -f "generators/conan_toolchain.cmake" ]; then
            TOOLCHAIN_FILE="generators/conan_toolchain.cmake"
        else
            print_error "Could not find conan_toolchain.cmake"
            exit 1
        fi
        local CMAKE_ARGS=(
            "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
            "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE"
        )
        if [ "$BUILD_TYPE" = "Debug" ]; then
            CMAKE_ARGS+=("-DENABLE_COVERAGE=ON")
        fi
        if [ "$VERBOSE" = true ]; then
            cmake "${CMAKE_ARGS[@]}" ..
        else
            cmake "${CMAKE_ARGS[@]}" .. > cmake-config.log 2>&1
            if [ $? -ne 0 ]; then
                print_error "CMake configuration failed. Check $BUILD_DIR/cmake-config.log"
                tail -n 20 cmake-config.log
                exit 1
            fi
        fi
    fi
    print_success "CMake configured"
}

build_project() {
    local TARGET="${1:-all}"
    print_step "Building $TARGET ($BUILD_TYPE mode)..."
    local ACTUAL_BUILD_DIR="$BUILD_DIR"
    
    if [ -f "$BUILD_DIR/$BUILD_TYPE/CMakeCache.txt" ]; then
        ACTUAL_BUILD_DIR="$BUILD_DIR/$BUILD_TYPE"
    elif [ -f "$BUILD_DIR/build/$BUILD_TYPE/CMakeCache.txt" ]; then
        ACTUAL_BUILD_DIR="$BUILD_DIR/build/$BUILD_TYPE"
    elif [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
        ACTUAL_BUILD_DIR="$BUILD_DIR"
    elif [ -f "CMakeUserPresets.json" ]; then
        print_warning "CMake presets detected but build not configured. Configuring..."
        configure_cmake
        if [ -f "$BUILD_DIR/$BUILD_TYPE/CMakeCache.txt" ]; then
            ACTUAL_BUILD_DIR="$BUILD_DIR/$BUILD_TYPE"
        elif [ -f "$BUILD_DIR/build/$BUILD_TYPE/CMakeCache.txt" ]; then
            ACTUAL_BUILD_DIR="$BUILD_DIR/build/$BUILD_TYPE"
        else
            print_error "Configuration failed. Run './r-type.sh rebuild' or configure manually."
            exit 1
        fi
    else
        print_error "No CMake cache found. Run './r-type.sh rebuild' to reconfigure."
        exit 1
    fi

    if ! cd "$ACTUAL_BUILD_DIR" 2>/dev/null; then
        print_error "Could not change to build directory: $ACTUAL_BUILD_DIR"
        exit 1
    fi
    local BUILD_ARGS=(
        "--build" "."
        "--config" "$BUILD_TYPE"
        "-j$JOBS"
    )
    if [ "$TARGET" != "all" ]; then
        BUILD_ARGS+=("--target" "$TARGET")
    fi
    if [ "$VERBOSE" = true ]; then
        cmake "${BUILD_ARGS[@]}"
    else
        cmake "${BUILD_ARGS[@]}" 2>&1 | grep -E "(Building|Linking|Error|error:|warning:)" || true
    fi
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        print_error "Build failed"
        exit 1
    fi
    print_success "Build completed"
}

clean_build() {
    print_step "Cleaning build directory..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    else
        print_warning "Build directory doesn't exist"
    fi
}

run_client() {
    print_step "Launching R-TYPE Client..."
    local BIN_DIR=$(get_bin_dir)
    if [ ! -f "$BIN_DIR/r-type_client" ]; then
        print_error "Client not found. Building..."
        do_build "r-type_client"
        BIN_DIR=$(get_bin_dir)
    fi
    cd "$BIN_DIR"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    ./r-type_client "$@"
}

run_server() {
    print_step "Launching R-TYPE Server..."
    local BIN_DIR=$(get_bin_dir)
    if [ ! -f "$BIN_DIR/r-type_server" ]; then
        print_error "Server not found. Building..."
        do_build "r-type_server"
        BIN_DIR=$(get_bin_dir)
    fi
    cd "$BIN_DIR"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    ./r-type_server "$@"
}

run_tests() {
    print_step "Running tests..."
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Building..."
        do_build
    fi
    local TEST_DIR="$BUILD_DIR"
    if [ -d "$BUILD_DIR/build/$BUILD_TYPE" ]; then
        TEST_DIR="$BUILD_DIR/build/$BUILD_TYPE"
    fi
    cd "$TEST_DIR"
    ctest --output-on-failure -C "$BUILD_TYPE"
    print_success "All tests passed!"
}

run_game_tests() {
    print_step "Running all unit tests..."
    local BIN_DIR=$(get_bin_dir)
    
    # Liste des tests à exécuter
    local TEST_BINS=("test_game" "test_network" "test_client_units")
    local ALL_PASSED=true
    
    for TEST_BIN in "${TEST_BINS[@]}"; do
        if [ ! -f "$BIN_DIR/$TEST_BIN" ]; then
            print_warning "$TEST_BIN not found, building..."
            do_build "$TEST_BIN"
            BIN_DIR=$(get_bin_dir)
        fi
        
        if [ -f "$BIN_DIR/$TEST_BIN" ]; then
            echo -e "\n${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            echo -e "${YELLOW}Running: $TEST_BIN${NC}"
            echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
            cd "$BIN_DIR"
            ./$TEST_BIN "$@"
            if [ $? -ne 0 ]; then
                ALL_PASSED=false
            fi
        fi
    done
    
    echo ""
    if [ "$ALL_PASSED" = true ]; then
        print_success "All test suites passed!"
    else
        print_error "Some test suites failed"
        exit 1
    fi
}

generate_coverage() {
    print_step "Generating code coverage report..."
    if [ "$BUILD_TYPE" != "Debug" ]; then
        print_warning "Coverage requires Debug build. Switching to Debug mode..."
        BUILD_TYPE="Debug"
    fi
    if ! command_exists gcov || ! command_exists lcov; then
        print_error "gcov/lcov not found. Install with: sudo dnf install lcov"
        echo "  or: sudo apt install lcov"
        exit 1
    fi
    do_build
    local TEST_DIR="$BUILD_DIR"
    if [ -d "$BUILD_DIR/build/$BUILD_TYPE" ]; then
        TEST_DIR="$BUILD_DIR/build/$BUILD_TYPE"
    fi
    cd "$TEST_DIR"
    ctest --output-on-failure -C Debug
    lcov --capture --directory . --output-file coverage.info
    lcov --remove coverage.info '/usr/*' '*/tests/*' '*/third_party/*' --output-file coverage.info
    lcov --list coverage.info
    if command_exists genhtml; then
        genhtml coverage.info --output-directory coverage_html
        print_success "Coverage report generated in $TEST_DIR/coverage_html/index.html"
        echo ""
        echo "Open the report with:"
        echo "  xdg-open $TEST_DIR/coverage_html/index.html"
    else
        print_warning "genhtml not found. Install lcov for HTML reports."
        print_success "Coverage data available in $TEST_DIR/coverage.info"
    fi
}

run_valgrind() {
    print_step "Running Valgrind memory analysis..."
    if ! command_exists valgrind; then
        print_error "Valgrind not found. Install with: sudo apt install valgrind"
        exit 1
    fi
    if [ "$BUILD_TYPE" != "Debug" ]; then
        print_warning "Valgrind works best with Debug build. Switching to Debug mode..."
        BUILD_TYPE="Debug"
        do_build
    fi
    cd "$BUILD_DIR"
    local TEST_BINS=$(find . -name "*test*" -type f -executable)
    if [ -z "$TEST_BINS" ]; then
        print_error "No test executables found"
        exit 1
    fi
    for TEST_BIN in $TEST_BINS; do
        echo -e "\n${YELLOW}Analyzing: $TEST_BIN${NC}"
        valgrind \
            --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --verbose \
            "$TEST_BIN"
    done
    print_success "Valgrind analysis completed"
}

do_build() {
    local TARGET="${1:-all}"
    if [ "$CLEAN_BUILD" = true ]; then
        clean_build
    fi
    check_conan
    check_cmake
    check_compiler
    check_dev_tools "$COMMAND"

    local TOOLCHAIN_EXISTS=false
    if [ -f "$BUILD_DIR/conan_toolchain.cmake" ] || \
       [ -f "$BUILD_DIR/build/$BUILD_TYPE/generators/conan_toolchain.cmake" ] || \
       [ -f "$BUILD_DIR/generators/conan_toolchain.cmake" ]; then
        TOOLCHAIN_EXISTS=true
    fi

    if [ "$TOOLCHAIN_EXISTS" = false ] || [ "$CLEAN_BUILD" = true ]; then
        install_dependencies
    fi

    local CMAKE_CONFIGURED=false
    if [ -f "$BUILD_DIR/CMakeCache.txt" ] || \
       [ -f "$BUILD_DIR/build/$BUILD_TYPE/CMakeCache.txt" ]; then
        CMAKE_CONFIGURED=true
    fi

    if [ "$CMAKE_CONFIGURED" = false ] || [ "$CLEAN_BUILD" = true ]; then
        configure_cmake
    fi

    build_project "$TARGET"
}

parse_options() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            -r|--release)
                BUILD_TYPE="Release"
                shift
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -j|--jobs)
                JOBS="$2"
                shift 2
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                break
                ;;
        esac
    done
}

main() {
    parse_options "$@"
    COMMAND="${1:-build}"
    shift || true
    case "$COMMAND" in
        build)
            print_banner
            do_build
            ;;
        client)
            print_banner
            do_build "r-type_client"
            run_client "$@"
            ;;
        server)
            print_banner
            do_build "r-type_server"
            run_server "$@"
            ;;
        test)
            print_banner
            do_build
            run_tests
            ;;
        tests)
            print_banner
            # Build tous les tests
            do_build "test_game"
            do_build "test_network"
            do_build "test_client_units"
            run_game_tests "$@"
            ;;
        coverage)
            print_banner
            generate_coverage
            ;;
        valgrind)
            print_banner
            run_valgrind
            ;;
        clean)
            print_banner
            clean_build
            ;;
        install)
            print_banner
            check_conan
            install_dependencies
            ;;
        rebuild)
            print_banner
            CLEAN_BUILD=true
            do_build
            ;;
        all)
            print_banner
            do_build "all"
            ;;
        -h|--help)
            show_help
            ;;
        *)
            print_error "Unknown command: $COMMAND"
            echo "Use './r-type.sh --help' for usage information"
            exit 1
            ;;
    esac
    echo ""
    print_success "Done !  "
}

main "$@"
