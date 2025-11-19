cmake_minimum_required(VERSION 3.20)

enable_testing()

add_executable(test_sanity test_sanity.cpp)
target_link_libraries(test_sanity PRIVATE
    gtest::gtest
    project_options
    project_warnings
)
add_test(NAME SanityTest COMMAND test_sanity)

message(STATUS "Tests configured (SanityTest enabled)")
