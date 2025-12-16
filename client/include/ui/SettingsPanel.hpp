#pragma once

#include "MenuComponents.hpp"
#include "../Settings.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>

namespace rtype::ui {

class SettingsPanel {
public:
    enum class Tab { Audio, Video, Controls };

    SettingsPanel(const sf::Vector2u& window_size);

    void handle_mouse_move(const sf::Vector2f& mouse_pos);
    void handle_mouse_click(const sf::Vector2f& mouse_pos);
    void handle_key_press(sf::Keyboard::Key key);
    void update(float dt);
    void render(sf::RenderWindow& window);

    bool is_open() const { return m_open; }
    void open() { m_open = true; m_temp_resolution_index = Settings::instance().resolution_index; m_temp_fullscreen = Settings::instance().fullscreen; m_temp_colorblind = Settings::instance().colorblind_mode; }
    void close() { m_open = false; }
    bool needs_window_recreate() const { return m_needs_window_recreate; }
    void clear_window_recreate_flag() { m_needs_window_recreate = false; }
    void get_new_window_settings(sf::Vector2u& size, bool& fullscreen) const;

private:
    void apply_settings();
    void switch_tab(Tab new_tab);
    void create_buttons();

    sf::RectangleShape m_overlay;
    sf::RectangleShape m_panel_bg;
    sf::Text m_title;
    sf::Font m_font;
    
    // Tab buttons
    std::vector<std::unique_ptr<::rtype::ui::Button>> m_tab_buttons;
    std::vector<sf::RectangleShape> m_tab_indicators;
    Tab m_current_tab{Tab::Audio};
    
    // Content buttons (change based on tab)
    std::vector<std::unique_ptr<::rtype::ui::Button>> m_buttons;
    
    // Display texts
    sf::Text m_volume_text;
    sf::Text m_resolution_text;
    sf::Text m_fullscreen_text;
    sf::Text m_colorblind_text;
    sf::Text m_controls_text;
    sf::RectangleShape m_volume_bar;
    
    bool m_open{false};
    sf::Vector2u m_window_size;
    size_t m_temp_resolution_index{0};
    bool m_temp_fullscreen{false};
    bool m_temp_colorblind{false};
    bool m_needs_window_recreate{false};
};

} // namespace rtype::ui
