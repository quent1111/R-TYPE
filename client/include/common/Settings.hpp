#pragma once

#include <vector>

struct Settings {
    int master_volume = 50;
    bool fullscreen = false;
    bool colorblind_mode = false;
    bool screen_shake_enabled = true;
    std::vector<std::pair<int, int>> resolutions{{1280, 720}, {1600, 900}, {1920, 1080}};
    size_t resolution_index = 2;

    static Settings& instance() {
        static Settings s;
        return s;
    }
};
