#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <map>

namespace rtype {

class SettingsManager {
public:
    static SettingsManager& get_instance();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    bool load_from_file(const std::string& filename = "settings.ini");
    bool save_to_file(const std::string& filename = "settings.ini");
    void set_resolution(unsigned int width, unsigned int height);
    sf::VideoMode get_resolution() const;
    unsigned int get_resolution_width() const { return m_resolution_width; }
    unsigned int get_resolution_height() const { return m_resolution_height; }
    void set_fullscreen(bool fullscreen);
    bool is_fullscreen() const { return m_fullscreen; }
    void set_vsync(bool vsync);
    bool is_vsync_enabled() const { return m_vsync; }
    void set_show_fps(bool show_fps);
    bool should_show_fps() const { return m_show_fps; }
    void set_graphics_quality(int quality);
    int get_graphics_quality() const { return m_graphics_quality; }
    void set_music_volume(int volume);
    int get_music_volume() const { return m_music_volume; }
    void set_sfx_volume(int volume);
    int get_sfx_volume() const { return m_sfx_volume; }
    void apply_to_window(sf::RenderWindow& window);
    void reset_to_defaults();

private:
    SettingsManager();
    ~SettingsManager() = default;
    void parse_line(const std::string& line);
    std::string trim(const std::string& str);
    unsigned int m_resolution_width;
    unsigned int m_resolution_height;
    bool m_fullscreen;
    bool m_vsync;
    bool m_show_fps;
    int m_graphics_quality;
    int m_music_volume;
    int m_sfx_volume;
    std::string m_settings_file;
};

} // namespace rtype
