#include "core/SettingsManager.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace rtype {

SettingsManager::SettingsManager()
    : m_resolution_width(1920)
    , m_resolution_height(1080)
    , m_fullscreen(false)
    , m_vsync(true)
    , m_show_fps(false)
    , m_graphics_quality(2)
    , m_music_volume(70)
    , m_sfx_volume(80)
    , m_settings_file("settings.ini")
{
}

SettingsManager& SettingsManager::get_instance() {
    static SettingsManager instance;
    return instance;
}

std::string SettingsManager::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void SettingsManager::parse_line(const std::string& line) {
    if (line.empty() || line[0] == '#' || line[0] == ';') {
        return;
    }

    size_t pos = line.find('=');
    if (pos == std::string::npos) {
        return;
    }

    std::string key = trim(line.substr(0, pos));
    std::string value = trim(line.substr(pos + 1));

    try {
        if (key == "resolution_width") {
            m_resolution_width = static_cast<unsigned int>(std::stoi(value));
        } else if (key == "resolution_height") {
            m_resolution_height = static_cast<unsigned int>(std::stoi(value));
        } else if (key == "fullscreen") {
            m_fullscreen = (value == "true" || value == "1");
        } else if (key == "vsync") {
            m_vsync = (value == "true" || value == "1");
        } else if (key == "show_fps") {
            m_show_fps = (value == "true" || value == "1");
        } else if (key == "graphics_quality") {
            m_graphics_quality = std::max(0, std::min(3, std::stoi(value)));
        } else if (key == "music_volume") {
            m_music_volume = std::max(0, std::min(100, std::stoi(value)));
        } else if (key == "sfx_volume") {
            m_sfx_volume = std::max(0, std::min(100, std::stoi(value)));
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "[SettingsManager] Warning: Invalid value for '" << key << "': '" << value
                  << "' (not a valid number). Using default.\n";
    } catch (const std::out_of_range& e) {
        std::cerr << "[SettingsManager] Warning: Value out of range for '" << key << "': '" << value
                  << "'. Using default.\n";
    }
}

bool SettingsManager::load_from_file(const std::string& filename) {
    m_settings_file = filename;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[SettingsManager] Could not open '" << filename << "', using defaults\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        parse_line(line);
    }

    file.close();
    std::cout << "[SettingsManager] Settings loaded from '" << filename << "'\n";
    return true;
}

bool SettingsManager::save_to_file(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[SettingsManager] Error: Could not save settings to '" << filename << "'\n";
        return false;
    }

    file << "# R-Type Settings File\n";
    file << "# Auto-generated - Edit with caution\n\n";
    file << "[Video]\n";
    file << "resolution_width=" << m_resolution_width << "\n";
    file << "resolution_height=" << m_resolution_height << "\n";
    file << "fullscreen=" << (m_fullscreen ? "true" : "false") << "\n";
    file << "vsync=" << (m_vsync ? "true" : "false") << "\n";
    file << "show_fps=" << (m_show_fps ? "true" : "false") << "\n";
    file << "graphics_quality=" << m_graphics_quality << "\n\n";
    file << "[Audio]\n";
    file << "music_volume=" << m_music_volume << "\n";
    file << "sfx_volume=" << m_sfx_volume << "\n";

    file.close();
    std::cout << "[SettingsManager] Settings saved to '" << filename << "'\n";
    return true;
}

void SettingsManager::set_resolution(unsigned int width, unsigned int height) {
    m_resolution_width = width;
    m_resolution_height = height;
}

sf::VideoMode SettingsManager::get_resolution() const {
    return sf::VideoMode(m_resolution_width, m_resolution_height);
}

void SettingsManager::set_fullscreen(bool fullscreen) {
    m_fullscreen = fullscreen;
}

void SettingsManager::set_vsync(bool vsync) {
    m_vsync = vsync;
}

void SettingsManager::set_show_fps(bool show_fps) {
    m_show_fps = show_fps;
}

void SettingsManager::set_graphics_quality(int quality) {
    m_graphics_quality = std::max(0, std::min(3, quality));
}

void SettingsManager::set_music_volume(int volume) {
    m_music_volume = std::max(0, std::min(100, volume));
}

void SettingsManager::set_sfx_volume(int volume) {
    m_sfx_volume = std::max(0, std::min(100, volume));
}

void SettingsManager::apply_to_window(sf::RenderWindow& window) {
    sf::VideoMode mode(m_resolution_width, m_resolution_height);
    sf::Uint32 style = m_fullscreen ? sf::Style::Fullscreen : sf::Style::Close;
    window.create(mode, "R-Type", style);
    window.setVerticalSyncEnabled(m_vsync);
    window.setFramerateLimit(60);
    std::cout << "[SettingsManager] Settings applied to window: "
              << m_resolution_width << "x" << m_resolution_height
              << " (Fullscreen: " << (m_fullscreen ? "ON" : "OFF")
              << ", VSync: " << (m_vsync ? "ON" : "OFF") << ")\n";
}

void SettingsManager::reset_to_defaults() {
    m_resolution_width = 1920;
    m_resolution_height = 1080;
    m_fullscreen = false;
    m_vsync = true;
    m_show_fps = false;
    m_graphics_quality = 2;
    m_music_volume = 70;
    m_sfx_volume = 80;
    std::cout << "[SettingsManager] Settings reset to defaults\n";
}

} // namespace rtype
