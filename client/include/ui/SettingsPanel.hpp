#pragma once

#include "MenuComponents.hpp"
#include "common/Settings.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>
#include <functional>

namespace rtype::ui {

class SettingsPanel {
public:
    enum class Tab { Game, Audio, Video, Controls };

    SettingsPanel(const sf::Vector2u& window_size);

    void handle_mouse_move(const sf::Vector2f& mouse_pos);
    void handle_mouse_click(const sf::Vector2f& mouse_pos);
    void handle_key_press(sf::Keyboard::Key key);
    void update(float dt);
    void render(sf::RenderWindow& window);

    bool is_open() const { return m_open; }
    void open() { open(false, 0); }
    void open(bool in_game, int connected_players) {
        m_open = true;
        m_in_game_mode = in_game;
        m_connected_players = connected_players;
        m_current_tab = (m_in_game_mode ? Tab::Game : Tab::Audio);
        m_temp_resolution_index = Settings::instance().resolution_index;
        m_temp_fullscreen = Settings::instance().fullscreen;
        m_temp_colorblind = Settings::instance().colorblind_mode;
        m_temp_screen_shake = Settings::instance().screen_shake_enabled;
        create_buttons();
    }
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

    std::vector<std::unique_ptr<::rtype::ui::Button>> m_tab_buttons;
    std::vector<sf::RectangleShape> m_tab_indicators;
    Tab m_current_tab{Tab::Audio};

    std::vector<std::unique_ptr<::rtype::ui::Button>> m_buttons;

    sf::Text m_volume_text;
    sf::Text m_resolution_text;
    sf::Text m_fullscreen_text;
    sf::Text m_colorblind_text;
    sf::Text m_controls_text;
    sf::RectangleShape m_volume_bar;

    bool m_open{false};
    sf::Vector2u m_window_size;
    bool m_in_game_mode{false};
    int m_connected_players{0};
    sf::Text m_players_text;
    sf::Text m_game_info_text;
    std::function<void()> m_quit_callback;
    size_t m_temp_resolution_index{0};
    bool m_temp_fullscreen{false};
    ColorBlindMode m_temp_colorblind{ColorBlindMode::Normal};
    bool m_temp_screen_shake{true};
    bool m_needs_window_recreate{false};
    int m_listening_control{-1};
    enum class ControlAction { Up = 0, Down = 1, Left = 2, Right = 3, Shoot = 4, Power1 = 5, Power2 = 6 };
public:
    void set_quit_callback(std::function<void()> cb) { m_quit_callback = std::move(cb); }
};

}  // namespace rtype::ui
