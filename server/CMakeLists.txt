cmake_minimum_required(VERSION 3.20)

add_executable(r-type_server
    src/main.cpp
    src/UDPServer.cpp
    src/Game.cpp
)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r-type_server PRIVATE
    r-type-engine
    project_options
    project_warnings
    asio::asio
)

link_platform_libraries(r-type_server)

add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:r-type_server>/assets
    COMMENT "Copying assets to build directory"
)

message(STATUS "Server executable configured: r-type_server")
