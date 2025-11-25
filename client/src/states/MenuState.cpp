#include "states/MenuState.hpp"

#include <iostream>

namespace rtype {

MenuState::MenuState(sf::RenderWindow& window) : m_window(window) {}

void MenuState::on_enter() {
    std::cout << "[MenuState] Entering menu\n";
    setup_ui();
}

void MenuState::on_exit() {
    std::cout << "[MenuState] Exiting menu\n";
    m_buttons.clear();
}

void MenuState::setup_ui() {
    auto window_size = m_window.getSize();
    sf::Vector2f center(static_cast<float>(window_size.x) / 2.0f,
                        static_cast<float>(window_size.y) / 2.0f);

    m_background = std::make_unique<ui::MenuBackground>(window_size);

    m_title = std::make_unique<ui::MenuTitle>(
        "R-TYPE", sf::Vector2f(center.x, center.y - 220.0f), 72);

    m_footer = std::make_unique<ui::MenuFooter>(window_size);

    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(30.0f, 30.0f), false, false));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(static_cast<float>(window_size.x) - 30.0f, 30.0f), true, false));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(30.0f, static_cast<float>(window_size.y) - 80.0f), false, true));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(static_cast<float>(window_size.x) - 30.0f,
                     static_cast<float>(window_size.y) - 80.0f), true, true));

    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(50.0f, center.y - 100.0f), true));
    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(static_cast<float>(window_size.x) - 50.0f, center.y - 100.0f), false));

    const float button_width = 340.0f;
    const float button_height = 70.0f;
    const float button_spacing = 90.0f;
    const float start_y = center.y - 10.0f;

    auto play_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y),
        sf::Vector2f(button_width, button_height), "PLAY");
    play_btn->set_colors(sf::Color(30, 80, 180, 220),
                         sf::Color(50, 120, 230, 255),
                         sf::Color(20, 60, 150, 255));
    play_btn->set_callback([this]() { on_play_clicked(); });
    m_buttons.push_back(std::move(play_btn));

    auto settings_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing),
        sf::Vector2f(button_width, button_height), "SETTINGS");
    settings_btn->set_colors(sf::Color(50, 50, 80, 200),
                             sf::Color(80, 80, 120, 255),
                             sf::Color(40, 40, 70, 255));
    settings_btn->set_callback([this]() { on_settings_clicked(); });
    m_buttons.push_back(std::move(settings_btn));

    auto quit_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f,
                     start_y + button_spacing * 2.0f),
        sf::Vector2f(button_width, button_height), "QUIT");
    quit_btn->set_colors(sf::Color(120, 30, 40, 200),
                         sf::Color(180, 50, 60, 255),
                         sf::Color(100, 20, 30, 255));
    quit_btn->set_callback([this]() { on_quit_clicked(); });
    m_buttons.push_back(std::move(quit_btn));
}

void MenuState::on_play_clicked() {
    std::cout << "[MenuState] Play button clicked - transitioning to game\n";
    m_next_state = "game";
}

void MenuState::on_settings_clicked() {
    std::cout << "[MenuState] Settings button clicked (not implemented yet)\n";
    m_next_state = "settings";
}

void MenuState::on_quit_clicked() {
    std::cout << "[MenuState] Quit button clicked\n";
    m_window.close();
}

void MenuState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        m_mouse_pos = sf::Vector2f(static_cast<float>(event.mouseMove.x),
                                    static_cast<float>(event.mouseMove.y));
        for (auto& button : m_buttons) {
            button->handle_mouse_move(m_mouse_pos);
        }
    } else if (event.type == sf::Event::MouseButtonPressed &&
               event.mouseButton.button == sf::Mouse::Left) {
        m_mouse_pos = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                    static_cast<float>(event.mouseButton.y));
        for (auto& button : m_buttons) {
            if (button->handle_mouse_click(m_mouse_pos)) {
                break;
            }
        }
    } else if (event.type == sf::Event::KeyPressed &&
               event.key.code == sf::Keyboard::Escape) {
        on_quit_clicked();
    }
}

void MenuState::update(float dt) {
    if (m_background) {
        m_background->update(dt);
    }
    if (m_title) {
        m_title->update(dt);
    }
    for (auto& corner : m_corners) {
        corner->update(dt);
    }
    for (auto& panel : m_side_panels) {
        panel->update(dt);
    }
    for (auto& button : m_buttons) {
        button->update(dt);
    }
}

void MenuState::render(sf::RenderWindow& window) {
    if (m_background) {
        m_background->render(window);
    }
    for (auto& corner : m_corners) {
        corner->render(window);
    }
    for (auto& panel : m_side_panels) {
        panel->render(window);
    }
    if (m_title) {
        m_title->render(window);
    }
    for (auto& button : m_buttons) {
        button->render(window);
    }
    if (m_footer) {
        m_footer->render(window);
    }
}

} // namespace rtype
