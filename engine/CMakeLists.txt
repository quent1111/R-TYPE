cmake_minimum_required(VERSION 3.20)

# R-Type Engine Library
# Contains ECS framework and shared engine components

add_library(r-type-engine INTERFACE)

target_include_directories(r-type-engine INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ECS headers are in engine/ecs/
# Usage: #include "ecs/registry.hpp"
# Available headers:
#   - ecs/entity.hpp
#   - ecs/registry.hpp
#   - ecs/sparse_array.hpp
#   - ecs/zipper.hpp
#   - ecs/zipper_iterator.hpp
#   - ecs/indexed_zipper.hpp
#   - ecs/components.hpp (base components: position, velocity, etc.)
#   - ecs/systems.hpp (example systems)

message(STATUS "Engine library configured with ECS framework")
