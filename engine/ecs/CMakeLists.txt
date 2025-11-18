# CMakeLists.txt for engine/ecs module

add_library(engine_ecs INTERFACE)

target_include_directories(engine_ecs INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Headers for IDE
file(GLOB_RECURSE ECS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ecs/*.hpp
)

target_sources(engine_ecs INTERFACE ${ECS_HEADERS})

# Installation (optional)
install(DIRECTORY include/ecs DESTINATION include)
