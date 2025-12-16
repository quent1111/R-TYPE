#pragma once

#include "states/IState.hpp"
#include "ui/MenuComponents.hpp"
#include "ui/SettingsPanel.hpp"
#include <memory>

#include <SFML/Graphics.hpp>

#include <memory>
#include <vector>

namespace rtype {


class MenuState : public IState {
public:
    explicit MenuState(sf::RenderWindow& window);
    ~MenuState() override = default;

    void on_enter() override;
    void on_exit() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    std::string get_next_state() const override { return m_next_state; }
    void clear_next_state() override { m_next_state.clear(); }

private:
    void setup_ui();
    void on_play_clicked();
    void on_quit_clicked();

    sf::RenderWindow& m_window;
    std::unique_ptr<ui::MenuBackground> m_background;
    std::unique_ptr<ui::MenuTitle> m_title;
    std::unique_ptr<ui::MenuFooter> m_footer;
    std::vector<std::unique_ptr<ui::Button>> m_buttons;
    std::vector<std::unique_ptr<ui::CornerDecoration>> m_corners;
    std::vector<std::unique_ptr<ui::SidePanel>> m_side_panels;
    std::unique_ptr<ui::SettingsPanel> m_settings_panel;
    std::string m_next_state;
    sf::Vector2f m_mouse_pos;
};

}  // namespace rtype
