# Game Logic Library
# This library contains all shared game logic (components, systems, entities)
# It will be used by both client and server

add_library(game_logic STATIC
    src/systems/input_system.cpp
    src/systems/movement_system.cpp
    src/systems/shooting_system.cpp
    src/systems/collision_system.cpp
    src/systems/cleanup_system.cpp
    src/entities/player_factory.cpp
    src/entities/enemy_factory.cpp
    src/entities/projectile_factory.cpp
    src/entities/explosion_factory.cpp
)

target_include_directories(game_logic
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/engine
)

target_link_libraries(game_logic
    PUBLIC
        r-type-engine
        project_options
        project_warnings
        sfml::sfml
)

target_compile_features(game_logic PUBLIC cxx_std_20)

message(STATUS "Game logic library configured")
