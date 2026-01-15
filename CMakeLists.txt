cmake_minimum_required(VERSION 3.20)
project(r-type VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Platform)
include(CompilerOptions)
include(CompilerWarnings)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo)" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

find_package(SFML QUIET COMPONENTS graphics window system audio)
if(NOT SFML_FOUND)
    find_package(SFML REQUIRED COMPONENTS graphics window system audio CONFIG)
endif()
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)
find_package(lz4 REQUIRED)
find_package(nlohmann_json REQUIRED)

if(TARGET sfml::sfml)
    message(STATUS "Using Conan SFML targets (sfml::sfml)")
    set(SFML_TARGETS sfml::sfml)
elseif(TARGET sfml-graphics)
    message(STATUS "Using system SFML targets (sfml-graphics, etc.)")
    add_library(sfml_unified INTERFACE)
    target_link_libraries(sfml_unified INTERFACE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
    )
    add_library(sfml::sfml ALIAS sfml_unified)
    set(SFML_TARGETS sfml::sfml)
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

add_subdirectory(game-lib)
add_subdirectory(engine)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(admin-client)

enable_testing()
add_subdirectory(tests)

message(STATUS "==========================================")
message(STATUS "R-Type Project Configuration v${PROJECT_VERSION}")
message(STATUS "==========================================")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "==========================================")
message(STATUS "Dependencies:")
message(STATUS "  - SFML: ${SFML_VERSION}")
message(STATUS "  - Boost: ${Boost_VERSION}")
message(STATUS "  - GTest: ${GTest_VERSION}")
message(STATUS "==========================================")
