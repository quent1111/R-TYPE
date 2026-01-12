cmake_minimum_required(VERSION 3.20)

project(r-type_admin VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

add_executable(r-type_admin
    src/main.cpp
    src/AdminClient.cpp
    src/AdminUI.cpp
    src/LoginScreen.cpp
)

target_include_directories(r-type_admin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(r-type_admin PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-network
)

if(UNIX AND NOT APPLE)
    target_link_libraries(r-type_admin PRIVATE pthread)
endif()
