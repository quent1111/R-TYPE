# CMakeLists.txt for server

add_executable(r-type_server
    src/main.cpp
)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r-type_server PRIVATE
    engine_ecs
    engine_net
    asio::asio
)

# Install binary
install(TARGETS r-type_server DESTINATION bin)
