cmake_minimum_required(VERSION 3.20)

add_executable(r-type_client
    src/main_menu.cpp
    src/game.cpp
    src/networkClient.cpp
    src/managers/AudioManager.cpp
    src/managers/EffectsManager.cpp
    src/managers/FontManager.cpp
    src/managers/TextureManager.cpp
    src/rendering/GameRenderer.cpp
    src/rendering/HUDRenderer.cpp
    src/rendering/OverlayRenderer.cpp
    src/states/StateManager.cpp
    src/states/MenuState.cpp
    src/states/LobbyState.cpp
    src/states/GameState.cpp
    src/ui/SettingsPanel.cpp
    src/ui/MenuComponents.cpp
)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(r-type_client PRIVATE
    project_options
    project_warnings
    sfml::sfml
    asio::asio
)

link_platform_libraries(r-type_client)

add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:r-type_client>/assets
    COMMENT "Copying assets to build directory"
)

message(STATUS "Client executable configured: r-type_client")
