cmake_minimum_required(VERSION 3.20)

enable_testing()

# ============================================================================
# Test Configuration
# ============================================================================
# All tests are currently placeholders waiting for implementation
# To implement a test:
#   1. Remove the DISABLED_ prefix from the test name
#   2. Add actual test logic
#   3. Uncomment the corresponding add_test() call below
# ============================================================================

# ----------------------------------------------------------------------------
# ECS (Entity-Component-System) Tests
# ----------------------------------------------------------------------------
add_executable(test_ecs
    ecs/test_entity.cpp
    ecs/test_component.cpp
    ecs/test_system.cpp
    ecs/test_registry.cpp
)
target_link_libraries(test_ecs PRIVATE
    r-type-engine
    gtest::gtest
    project_options
    project_warnings
)
# add_test(NAME ECSTests COMMAND test_ecs)

# ----------------------------------------------------------------------------
# Network Tests
# ----------------------------------------------------------------------------
add_executable(test_network
    network/test_main_network.cpp
    network/test_protocol.cpp
    network/test_serialization.cpp
    network/test_udp_socket.cpp
    # Advanced tests
    network/test_serialization_advanced.cpp
    network/test_protocol_advanced.cpp
    network/test_opcodes.cpp
    network/test_serializer_edge_cases.cpp
    # Compression tests
    network/test_compression.cpp
    network/test_full_compression_integration.cpp
)

target_include_directories(test_network PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/game-lib/include
)

target_link_libraries(test_network PRIVATE
    r-type-engine
    gtest::gtest
    asio::asio
    LZ4::lz4_static
    project_options
    project_warnings
)
add_test(NAME NetworkTests COMMAND test_network)

# ----------------------------------------------------------------------------
# Game Logic Tests
# ----------------------------------------------------------------------------
add_executable(test_game
    game/test_main.cpp
    game/test_collision.cpp
    game/test_movement.cpp
    game/test_spawning.cpp
    game/test_level_manager.cpp
    game/test_health.cpp
    game/test_shield.cpp
    game/test_weapon.cpp
    game/test_power_cannon.cpp
    # Advanced tests
    game/test_collision_components.cpp
    game/test_movement_components.cpp
    game/test_weapon_advanced.cpp
    game/test_powerups.cpp
    game/test_health_advanced.cpp
    game/test_tags.cpp
    game/test_edge_cases.cpp
)

target_include_directories(test_game PRIVATE
    ${CMAKE_SOURCE_DIR}/game-lib/include
)

target_link_libraries(test_game PRIVATE
    r-type-engine
    gtest::gtest
    project_options
    project_warnings
)
# add_test(NAME GameTests COMMAND test_game)

# ----------------------------------------------------------------------------
# Rendering Tests
# ----------------------------------------------------------------------------
add_executable(test_render
    render/test_sprite_system.cpp
    render/test_animation.cpp
)
target_link_libraries(test_render PRIVATE
    r-type-engine
    gtest::gtest
    sfml::sfml
    project_options
    project_warnings
)
# add_test(NAME RenderTests COMMAND test_render)

# ----------------------------------------------------------------------------
# Integration Tests
# ----------------------------------------------------------------------------
add_executable(test_integration
    integration/test_client_server.cpp
    integration/test_multiplayer.cpp
)
target_link_libraries(test_integration PRIVATE
    r-type-engine
    gtest::gtest
    asio::asio
    project_options
    project_warnings
)
# add_test(NAME IntegrationTests COMMAND test_integration)

# ----------------------------------------------------------------------------
# Client Tests (Settings and State Management)
# ----------------------------------------------------------------------------
add_executable(test_client_minimal
    client/test_minimal.cpp
)

target_link_libraries(test_client_minimal PRIVATE
    gtest::gtest
    project_options
    project_warnings
)

add_test(NAME ClientMinimalTest COMMAND test_client_minimal)

# ----------------------------------------------------------------------------
# Client Unit Tests (Entity, SafeQueue)
# ----------------------------------------------------------------------------
add_executable(test_client_units
    client/test_main_client.cpp
    client/test_entity.cpp
    client/test_safe_queue.cpp
    client/test_entity_advanced.cpp
    client/test_queue_advanced.cpp
)

target_include_directories(test_client_units PRIVATE
    ${CMAKE_SOURCE_DIR}/client/src
    ${CMAKE_SOURCE_DIR}/client/include
)

target_link_libraries(test_client_units PRIVATE
    gtest::gtest
    sfml::sfml
    project_options
    project_warnings
)

add_test(NAME ClientUnitsTest COMMAND test_client_units)

# Je remettrai quand j'aurai mis le menu
# add_executable(test_client
#     client/test_settings_manager.cpp
#     client/test_state_manager.cpp
#     ${CMAKE_SOURCE_DIR}/client/src/core/SettingsManager.cpp
#     ${CMAKE_SOURCE_DIR}/client/src/core/FontManager.cpp
#     ${CMAKE_SOURCE_DIR}/client/src/states/StateManager.cpp
# )
#
# target_include_directories(test_client PRIVATE
#     ${CMAKE_SOURCE_DIR}/client/include
# )
#
# target_link_libraries(test_client PRIVATE
#     gtest::gtest
#     sfml::sfml
#     project_options
#     project_warnings
# )
#
# add_test(NAME ClientTests COMMAND test_client)

# ----------------------------------------------------------------------------
# Sanity Test (Always Enabled)
# ----------------------------------------------------------------------------
add_executable(test_sanity test_sanity.cpp)
target_link_libraries(test_sanity PRIVATE
    gtest::gtest
    project_options
    project_warnings
)
add_test(NAME SanityTest COMMAND test_sanity)

message(STATUS "==========================================")
message(STATUS "Test Suite Configuration")
message(STATUS "==========================================")
message(STATUS "✓ Sanity tests: ENABLED")
message(STATUS "✓ Client tests: ENABLED")
message(STATUS "⏸ ECS tests: DISABLED (placeholder)")
message(STATUS "⏸ Network tests: DISABLED (placeholder)")
message(STATUS "⏸ Game tests: DISABLED (placeholder)")
message(STATUS "⏸ Render tests: DISABLED (placeholder)")
message(STATUS "⏸ Integration tests: DISABLED (placeholder)")
message(STATUS "==========================================")
message(STATUS "To enable tests, uncomment add_test() calls")
message(STATUS "and implement test logic in respective files")
message(STATUS "==========================================")
