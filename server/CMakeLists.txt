cmake_minimum_required(VERSION 3.20)

add_executable(r-type_server
    src/main.cpp
    ../src/Common/Opcodes.cpp
    src/network/UDPServer.cpp
    src/network/EntityBroadcaster.cpp
    src/network/LobbyBroadcaster.cpp
    src/network/GameBroadcaster.cpp
    src/network/PowerupBroadcaster.cpp
    src/handlers/InputHandler.cpp
    src/handlers/PowerupHandler.cpp
    src/handlers/WeaponHandler.cpp
    src/handlers/LobbyCommandHandler.cpp
    src/game/PlayerManager.cpp
    src/game/LevelManager.cpp
    src/game/BossManager.cpp
    src/game/GameSession.cpp
    src/game/Lobby.cpp
    src/game/LobbyManager.cpp
    src/game/ServerCore.cpp
)

target_include_directories(r-type_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/game-lib/include
)

target_link_libraries(r-type_server PRIVATE
    r-type-engine
    game_logic
    project_options
    project_warnings
    asio::asio
    LZ4::lz4_static
)

link_platform_libraries(r-type_server)

add_custom_command(TARGET r-type_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:r-type_server>/assets
    COMMENT "Copying assets to build directory"
)

message(STATUS "Server executable configured: r-type_server")
