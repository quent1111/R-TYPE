
find_package(nlohmann_json REQUIRED)

add_library(game_logic STATIC
    src/systems/input_system.cpp
    src/systems/movement_system.cpp
    src/systems/shooting_system.cpp
    src/systems/collision_system.cpp
    src/systems/cleanup_system.cpp
    src/systems/wave_system.cpp
    src/systems/custom_wave_system.cpp
    src/systems/explosive_system.cpp
    src/entities/player_factory.cpp
    src/entities/enemy_factory.cpp
    src/entities/boss_factory.cpp
    src/entities/projectile_factory.cpp
    src/entities/explosion_factory.cpp
    src/powerup/PowerupRegistry.cpp
    src/powerup/PowerupCardPool.cpp
    src/level/LevelConfigParser.cpp
    src/level/CustomLevelManager.cpp
    src/level/LevelEntityFactory.cpp
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
        nlohmann_json::nlohmann_json
)

target_compile_features(game_logic PUBLIC cxx_std_20)

message(STATUS "Game logic library configured")
