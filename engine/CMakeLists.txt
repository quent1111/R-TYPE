cmake_minimum_required(VERSION 3.20)

# R-Type Engine Library
# Contains ECS framework, system manager, and game engine core

add_library(r-type-engine STATIC
    core/SystemManager.cpp
    core/GameEngine.cpp
)

target_include_directories(r-type-engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ECS headers are in engine/ecs/
# Core engine headers are in engine/core/
# Usage: #include "ecs/registry.hpp" or #include "core/GameEngine.hpp"
# Available headers:
#   ECS:
#   - ecs/entity.hpp
#   - ecs/registry.hpp
#   - ecs/sparse_array.hpp
#   - ecs/zipper.hpp
#   - ecs/zipper_iterator.hpp
#   - ecs/indexed_zipper.hpp
#   - ecs/components.hpp (base components: position, velocity, etc.)
#   
#   Core Engine:
#   - core/ISystem.hpp (interface for game systems)
#   - core/SystemManager.hpp (manages system lifecycle)
#   - core/GameEngine.hpp (main engine class)

message(STATUS "Engine library configured with ECS framework and modular system architecture")
