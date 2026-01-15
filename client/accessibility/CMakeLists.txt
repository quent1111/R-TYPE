cmake_minimum_required(VERSION 3.15)

# Module d'accessibilité pour R-TYPE
# Ce module est indépendant et peut être compilé séparément

project(rtype-accessibility CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouver SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Sources du module d'accessibilité
set(ACCESSIBILITY_SOURCES
    src/ColorTransform.cpp
    src/AccessibilityManager.cpp
    src/ProjectileShapeRenderer.cpp
)

set(ACCESSIBILITY_HEADERS
    include/ColorBlindnessMode.hpp
    include/ColorTransform.hpp
    include/AccessibilityManager.hpp
    include/ProjectileShapeRenderer.hpp
)

# Créer une bibliothèque statique
add_library(rtype-accessibility STATIC
    ${ACCESSIBILITY_SOURCES}
    ${ACCESSIBILITY_HEADERS}
)

# Inclure les headers
target_include_directories(rtype-accessibility PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Lier SFML (utiliser la même interface que le projet principal)
target_link_libraries(rtype-accessibility PUBLIC
    sfml::sfml
)

# Options de compilation
if(MSVC)
    target_compile_options(rtype-accessibility PRIVATE /W4)
else()
    target_compile_options(rtype-accessibility PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation (optionnel)
install(TARGETS rtype-accessibility
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${ACCESSIBILITY_HEADERS}
    DESTINATION include/accessibility
)

# Message de fin
message(STATUS "✓ R-TYPE Accessibility Module configured")
message(STATUS "  - Color blindness support: Protanopia, Deuteranopia, Tritanopia, High Contrast")
message(STATUS "  - Projectile shape differentiation enabled")
message(STATUS "  - WCAG AAA contrast ratio support")
