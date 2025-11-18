cmake_minimum_required(VERSION 3.18)
project(r-type VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build example games" OFF)

# Find dependencies (Conan provides these via CMakeDeps)
find_package(SFML REQUIRED COMPONENTS graphics audio window system network)
find_package(asio REQUIRED)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
endif()

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Engine modules (header-only for now)
add_subdirectory(engine/ecs)
# add_subdirectory(engine/net)    # TODO: implement
# add_subdirectory(engine/render) # TODO: implement
# add_subdirectory(engine/audio)  # TODO: implement
# add_subdirectory(engine/core)   # TODO: implement
# add_subdirectory(engine/utils)  # TODO: implement

# Placeholder libs for client/server linking
add_library(engine_net INTERFACE)
add_library(engine_render INTERFACE)
add_library(engine_audio INTERFACE)

# Client and Server executables
add_subdirectory(client)
add_subdirectory(server)

# Tests
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Print configuration summary
message(STATUS "====================================")
message(STATUS "R-Type Configuration Summary")
message(STATUS "====================================")
message(STATUS "Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:     C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests:      ${BUILD_TESTS}")
message(STATUS "Build examples:   ${BUILD_EXAMPLES}")
message(STATUS "Install prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "====================================")

