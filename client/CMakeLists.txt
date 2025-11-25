cmake_minimum_required(VERSION 3.20)

# Original game (without menu)
add_executable(r-type_client_old
    src/main.cpp
)

target_include_directories(r-type_client_old PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r-type_client_old PRIVATE
    game_logic
    r-type-engine
    project_options
    project_warnings
    sfml::sfml
)

link_platform_libraries(r-type_client_old)

# New client with menu system
add_executable(r-type_client
    src/main_menu.cpp
    src/states/StateManager.cpp
    src/states/MenuState.cpp
    src/states/GameState.cpp
    src/ui/MenuComponents.cpp
)

target_include_directories(r-type_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(r-type_client PRIVATE
    game_logic
    r-type-engine
    project_options
    project_warnings
    sfml::sfml
)

link_platform_libraries(r-type_client)

add_custom_command(TARGET r-type_client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:r-type_client>/assets
    COMMENT "Copying assets to build directory"
)

message(STATUS "Client executable configured: r-type_client")
