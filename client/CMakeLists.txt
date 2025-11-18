# CMakeLists.txt for client

add_executable(r-type_client
    src/main.cpp
)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r-type_client PRIVATE
    engine_ecs
    engine_net
    engine_render
    engine_audio
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Network
)

# Install binary
install(TARGETS r-type_client DESTINATION bin)
