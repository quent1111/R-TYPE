cmake_minimum_required(VERSION 3.20)

add_library(r-type-engine INTERFACE)

target_include_directories(r-type-engine INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

message(STATUS "Engine library configured")
