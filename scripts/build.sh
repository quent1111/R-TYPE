#!/bin/bash
# Bootstrap script for R-Type project

set -e

echo "======================================"
echo "R-Type Project Bootstrap"
echo "======================================"

# Check if conan is installed
if ! command -v conan &> /dev/null; then
    echo "❌ Conan not found. Installing..."
    pip3 install conan
else
    echo "✅ Conan found"
fi

# Detect conan profile
echo "🔍 Detecting Conan profile..."
conan profile detect --force

# Install dependencies
echo "📦 Installing dependencies..."
conan install . --output-folder=build --build=missing

# Configure CMake
echo "⚙️  Configuring CMake..."
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake

# Build
echo "🔨 Building project..."
cmake --build build --config Release -j$(nproc)

echo ""
echo "======================================"
echo "✅ Build complete!"
echo "======================================"
echo ""
echo "Run the server: ./build/bin/r-type_server"
echo "Run the client: ./build/bin/r-type_client"
echo "Run tests:      cd build && ctest"
