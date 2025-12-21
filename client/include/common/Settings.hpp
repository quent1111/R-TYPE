#pragma once

#include <vector>
#include <SFML/Window.hpp>

struct Settings {
    int master_volume = 50;
    bool fullscreen = false;
    bool colorblind_mode = false;
    bool screen_shake_enabled = true;
    std::vector<std::pair<int, int>> resolutions{{1280, 720}, {1600, 900}, {1920, 1080}};
    size_t resolution_index = 2;

    int key_up = static_cast<int>(sf::Keyboard::Z);
    int key_down = static_cast<int>(sf::Keyboard::S);
    int key_left = static_cast<int>(sf::Keyboard::Q);
    int key_right = static_cast<int>(sf::Keyboard::D);
    int key_shoot = static_cast<int>(sf::Keyboard::Space);
    int key_powerup1 = static_cast<int>(sf::Keyboard::A);
    int key_powerup2 = static_cast<int>(sf::Keyboard::E);

    static Settings& instance() {
        static Settings s;
        return s;
    }
};
