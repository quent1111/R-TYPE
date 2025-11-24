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
    network/test_protocol.cpp
    network/test_serialization.cpp
    network/test_udp_socket.cpp
)
target_link_libraries(test_network PRIVATE
    r-type-engine
    gtest::gtest
    asio::asio
    project_options
    project_warnings
)
# add_test(NAME NetworkTests COMMAND test_network)

# ----------------------------------------------------------------------------
# Game Logic Tests
# ----------------------------------------------------------------------------
add_executable(test_game
    game/test_collision.cpp
    game/test_movement.cpp
    game/test_spawning.cpp
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
    sfml-graphics
    sfml-window
    sfml-system
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
message(STATUS "⏸ ECS tests: DISABLED (placeholder)")
message(STATUS "⏸ Network tests: DISABLED (placeholder)")
message(STATUS "⏸ Game tests: DISABLED (placeholder)")
message(STATUS "⏸ Render tests: DISABLED (placeholder)")
message(STATUS "⏸ Integration tests: DISABLED (placeholder)")
message(STATUS "==========================================")
message(STATUS "To enable tests, uncomment add_test() calls")
message(STATUS "and implement test logic in respective files")
message(STATUS "==========================================")
