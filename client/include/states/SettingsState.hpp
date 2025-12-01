#pragma once

#include "states/IState.hpp"
#include "ui/MenuComponents.hpp"

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>

namespace rtype {

class SettingsState : public IState {
public:
    explicit SettingsState(sf::RenderWindow& window);
    ~SettingsState() override = default;

    void on_enter() override;
    void on_exit() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    std::string get_next_state() const override { return m_next_state; }
    void clear_next_state() override { m_next_state.clear(); }

private:
    void setup_ui();
    void on_back_clicked();
    void on_apply_clicked();
    void on_reset_defaults_clicked();
    void toggle_fullscreen();
    void toggle_vsync();
    void toggle_show_fps();
    void change_resolution(int direction);
    void change_volume(const std::string& type, int direction);
    void change_quality(int direction);
    void update_settings_display();
    void apply_settings_immediately();
    void recreate_window_with_settings();

    sf::RenderWindow& m_window;
    std::unique_ptr<ui::MenuBackground> m_background;
    std::unique_ptr<ui::MenuTitle> m_title;
    std::vector<std::unique_ptr<ui::Button>> m_buttons;
    std::string m_next_state;
    sf::Vector2f m_mouse_pos;

    bool m_fullscreen;
    bool m_vsync;
    bool m_show_fps;
    int m_resolution_index;
    int m_music_volume;
    int m_sfx_volume;
    int m_quality_index;
    std::vector<sf::VideoMode> m_available_resolutions;
    std::vector<sf::Text> m_settings_labels;
    std::vector<sf::Text> m_settings_values;
    bool m_window_settings_changed;
};

} // namespace rtype
