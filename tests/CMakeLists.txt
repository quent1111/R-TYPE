# CMakeLists.txt for tests

if(NOT BUILD_TESTS)
    return()
endif()

# ECS tests
add_executable(ecs_tests
    ecs/test_entity.cpp
    ecs/test_sparse_array.cpp
    ecs/test_registry.cpp
)

target_link_libraries(ecs_tests PRIVATE
    engine_ecs
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME ecs_tests COMMAND ecs_tests)

# TODO: Add more test suites (net_tests, integration_tests, etc.)
