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

    // Background
    m_background = std::make_unique<ui::MenuBackground>(window_size);

    // Title
    m_title = std::make_unique<ui::MenuTitle>(
        "R-TYPE", sf::Vector2f(center.x, center.y - 200.0f), 72);

    // Buttons setup
    const float button_width = 300.0f;
    const float button_height = 60.0f;
    const float button_spacing = 80.0f;
    const float start_y = center.y - 50.0f;

    // Play button
    auto play_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y),
        sf::Vector2f(button_width, button_height), "PLAY");
    play_btn->set_colors(sf::Color(50, 100, 200),   // normal
                         sf::Color(70, 120, 220),   // hover
                         sf::Color(30, 80, 180));   // pressed
    play_btn->set_callback([this]() { on_play_clicked(); });
    m_buttons.push_back(std::move(play_btn));

    // Settings button
    auto settings_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing),
        sf::Vector2f(button_width, button_height), "SETTINGS");
    settings_btn->set_colors(sf::Color(70, 70, 70),   // normal
                             sf::Color(100, 100, 100), // hover
                             sf::Color(50, 50, 50));   // pressed
    settings_btn->set_callback([this]() { on_settings_clicked(); });
    m_buttons.push_back(std::move(settings_btn));

    // Quit button
    auto quit_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f,
                     start_y + button_spacing * 2.0f),
        sf::Vector2f(button_width, button_height), "QUIT");
    quit_btn->set_colors(sf::Color(150, 50, 50),   // normal
                         sf::Color(180, 70, 70),   // hover
                         sf::Color(120, 30, 30));  // pressed
    quit_btn->set_callback([this]() { on_quit_clicked(); });
    m_buttons.push_back(std::move(quit_btn));
}

void MenuState::on_play_clicked() {
    std::cout << "[MenuState] Play button clicked - transitioning to game\n";
    m_next_state = "game";
}

void MenuState::on_settings_clicked() {
    std::cout << "[MenuState] Settings button clicked (not implemented yet)\n";
    // Future: m_next_state = "settings";
}

void MenuState::on_quit_clicked() {
    std::cout << "[MenuState] Quit button clicked\n";
    m_window.close();
}

void MenuState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        m_mouse_pos = sf::Vector2f(static_cast<float>(event.mouseMove.x),
                                    static_cast<float>(event.mouseMove.y));
        
        // Update all buttons with mouse position
        for (auto& button : m_buttons) {
            button->handle_mouse_move(m_mouse_pos);
        }
    } else if (event.type == sf::Event::MouseButtonPressed &&
               event.mouseButton.button == sf::Mouse::Left) {
        m_mouse_pos = sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                    static_cast<float>(event.mouseButton.y));
        
        // Check button clicks
        for (auto& button : m_buttons) {
            if (button->handle_mouse_click(m_mouse_pos)) {
                break; // Only one button can be clicked at a time
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
    
    for (auto& button : m_buttons) {
        button->update(dt);
    }
}

void MenuState::render(sf::RenderWindow& window) {
    if (m_background) {
        m_background->render(window);
    }
    
    if (m_title) {
        m_title->render(window);
    }
    
    for (auto& button : m_buttons) {
        button->render(window);
    }
}

} // namespace rtype
