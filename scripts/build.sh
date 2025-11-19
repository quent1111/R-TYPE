#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}R-Type Project Build Script${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

if ! command -v conan &> /dev/null; then
    echo -e "${RED}Error: Conan is not installed!${NC}"
    echo -e "${YELLOW}Install it with: pip install conan${NC}"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed!${NC}"
    echo -e "${YELLOW}Install it from: https://cmake.org/download/${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Conan found: $(conan --version)${NC}"
echo -e "${GREEN}✓ CMake found: $(cmake --version | head -n1)${NC}"
echo ""

if [ ! -f "$HOME/.conan2/profiles/default" ]; then
    echo -e "${YELLOW}⚙ Conan profile not found, creating one...${NC}"
    conan profile detect --force
    echo -e "${GREEN}✓ Conan profile created${NC}"
    echo ""
fi

echo -e "${BLUE}[1/5] Checking system dependencies...${NC}"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    MISSING_DEPS=()
    
    for pkg in libxcb-devel libfontenc-devel libXaw-devel libXcomposite-devel \
               libXdmcp-devel libXtst-devel libxkbfile-devel libXres-devel \
               libXScrnSaver-devel xcb-util-wm-devel xcb-util-keysyms-devel \
               xcb-util-renderutil-devel libXdamage-devel libXv-devel \
               xcb-util-devel libuuid-devel xcb-util-cursor-devel; do
        if ! rpm -q $pkg &> /dev/null; then
            MISSING_DEPS+=($pkg)
        fi
    done
    
    if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
        echo -e "${YELLOW}⚠ Missing X11 development libraries for SFML${NC}"
        echo -e "${YELLOW}The following packages are required:${NC}"
        echo -e "${YELLOW}  ${MISSING_DEPS[*]}${NC}"
        echo ""
        echo -e "${BLUE}Install them with:${NC}"
        if command -v dnf &> /dev/null; then
            echo -e "${GREEN}sudo dnf install -y ${MISSING_DEPS[*]}${NC}"
        elif command -v apt &> /dev/null; then
            echo -e "${GREEN}sudo apt install -y libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev${NC}"
        fi
        echo ""
        read -p "Install automatically? (y/N) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            if command -v dnf &> /dev/null; then
                sudo dnf install -y ${MISSING_DEPS[*]}
            elif command -v apt &> /dev/null; then
                sudo apt install -y libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
            fi
            echo -e "${GREEN}✓ System dependencies installed${NC}"
        else
            echo -e "${RED}Please install the dependencies manually and re-run this script${NC}"
            exit 1
        fi
    else
        echo -e "${GREEN}✓ All system dependencies present${NC}"
    fi
else
    echo -e "${GREEN}✓ Non-Linux system, skipping X11 check${NC}"
fi
echo ""

echo -e "${BLUE}[2/5] Installing dependencies with Conan...${NC}"
conan install . --output-folder=$BUILD_DIR --build=missing -s build_type=$BUILD_TYPE
echo -e "${GREEN}✓ Dependencies installed${NC}"
echo ""

echo -e "${BLUE}[3/5] Configuring CMake...${NC}"
cmake -B $BUILD_DIR -S . \
    -DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/build/$BUILD_TYPE/generators/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
echo -e "${GREEN}✓ CMake configured${NC}"
echo ""

echo -e "${BLUE}[4/5] Building the project...${NC}"
cmake --build $BUILD_DIR --config $BUILD_TYPE -j$(nproc 2>/dev/null || echo 4)
echo -e "${GREEN}✓ Build complete${NC}"
echo ""

echo -e "${BLUE}[5/5] Build Summary${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}✓ Build completed successfully!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "${YELLOW}Executables location:${NC}"
echo -e "  Server: ${GREEN}$BUILD_DIR/bin/r-type_server${NC}"
echo -e "  Client: ${GREEN}$BUILD_DIR/bin/r-type_client${NC}"
echo ""
echo -e "${YELLOW}To run the server:${NC}"
echo -e "  cd $BUILD_DIR/bin && ./r-type_server"
echo ""
echo -e "${YELLOW}To run the client:${NC}"
echo -e "  cd $BUILD_DIR/bin && ./r-type_client"
echo ""
echo -e "${YELLOW}To run tests:${NC}"
echo -e "  cd $BUILD_DIR && ctest --output-on-failure"
echo ""
echo -e "${BLUE}========================================${NC}"
