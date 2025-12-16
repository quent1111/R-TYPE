#include "states/MenuState.hpp"

#include "AudioManager.hpp"
#include "ui/SettingsPanel.hpp"

#include <iostream>

namespace rtype {

MenuState::MenuState(sf::RenderWindow& window) : m_window(window) {}

void MenuState::on_enter() {
    std::cout << "[MenuState] Entering menu\n";
    m_buttons.clear();
    m_corners.clear();
    m_side_panels.clear();
    m_background.reset();
    m_title.reset();
    m_footer.reset();
    setup_ui();

    auto& audio = AudioManager::getInstance();
    audio.loadSounds();
    audio.playMusic("assets/sounds/menu-loop.ogg", true);
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

    m_title =
        std::make_unique<ui::MenuTitle>("R-TYPE", sf::Vector2f(center.x, center.y - 220.0f), 72);

    m_footer = std::make_unique<ui::MenuFooter>(window_size);

    m_corners.push_back(
        std::make_unique<ui::CornerDecoration>(sf::Vector2f(30.0f, 30.0f), false, false));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(static_cast<float>(window_size.x) - 30.0f, 30.0f), true, false));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(30.0f, static_cast<float>(window_size.y) - 80.0f), false, true));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(static_cast<float>(window_size.x) - 30.0f,
                     static_cast<float>(window_size.y) - 80.0f),
        true, true));

    m_side_panels.push_back(
        std::make_unique<ui::SidePanel>(sf::Vector2f(50.0f, center.y - 100.0f), true));
    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(static_cast<float>(window_size.x) - 50.0f, center.y - 100.0f), false));

    const float button_width = 340.0f;
    const float button_height = 70.0f;
    const float button_spacing = 90.0f;
    const float start_y = center.y - 10.0f;

    auto play_btn =
        std::make_unique<ui::Button>(sf::Vector2f(center.x - button_width / 2.0f, start_y),
                                     sf::Vector2f(button_width, button_height), "PLAY");
    play_btn->set_colors(sf::Color(30, 80, 180, 220), sf::Color(50, 120, 230, 255),
                         sf::Color(20, 60, 150, 255));
    play_btn->set_callback([this]() { on_play_clicked(); });
    m_buttons.push_back(std::move(play_btn));
    auto settings_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing),
        sf::Vector2f(button_width, button_height), "SETTINGS");
    settings_btn->set_colors(sf::Color(200, 120, 30, 220), sf::Color(230, 160, 50, 255),
                             sf::Color(160, 90, 20, 255));
    settings_btn->set_callback([this]() {
        if (!m_settings_panel) {
            m_settings_panel = std::make_unique<ui::SettingsPanel>(m_window.getSize());
        }
        m_settings_panel->open();
    });
    m_buttons.push_back(std::move(settings_btn));

    auto quit_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing * 2.0f),
        sf::Vector2f(button_width, button_height), "QUIT");
    quit_btn->set_colors(sf::Color(120, 30, 40, 200), sf::Color(180, 50, 60, 255),
                         sf::Color(100, 20, 30, 255));
    quit_btn->set_callback([this]() { on_quit_clicked(); });
    m_buttons.push_back(std::move(quit_btn));
}

void MenuState::on_play_clicked() {
    std::cout << "[MenuState] Play button clicked\n";
    AudioManager::getInstance().playSound(AudioManager::SoundType::Plop);
    m_next_state = "lobby";
}

void MenuState::on_quit_clicked() {
    std::cout << "[MenuState] Quit button clicked\n";
    AudioManager::getInstance().playSound(AudioManager::SoundType::Plop);
    m_window.close();
}

void MenuState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2i pixel_pos(event.mouseMove.x, event.mouseMove.y);
        m_mouse_pos = m_window.mapPixelToCoords(pixel_pos);
        if (m_settings_panel && m_settings_panel->is_open()) {
            m_settings_panel->handle_mouse_move(m_mouse_pos);
        } else {
            for (auto& button : m_buttons) {
                button->handle_mouse_move(m_mouse_pos);
            }
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i pixel_pos(event.mouseButton.x, event.mouseButton.y);
            sf::Vector2f click_pos = m_window.mapPixelToCoords(pixel_pos);
            if (m_settings_panel && m_settings_panel->is_open()) {
                m_settings_panel->handle_mouse_click(click_pos);
            } else {
                for (auto& button : m_buttons) {
                    button->handle_mouse_click(click_pos);
                }
            }
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            m_window.close();
        }
        if (event.key.code == sf::Keyboard::S) {
            if (!m_settings_panel) m_settings_panel = std::make_unique<ui::SettingsPanel>(m_window.getSize());
            if (m_settings_panel->is_open()) m_settings_panel->close();
            else m_settings_panel->open();
        }
        if (m_settings_panel && m_settings_panel->is_open()) {
            if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right) {
                m_settings_panel->handle_key_press(event.key.code);
            }
        }
    }
}

void MenuState::update(float dt) {
    if (m_background) {
        m_background->update(dt);
    }
    if (m_title) {
        m_title->update(dt);
    }
    for (auto& button : m_buttons) {
        button->update(dt);
    }
    for (auto& corner : m_corners) {
        corner->update(dt);
    }
    for (auto& panel : m_side_panels) {
        panel->update(dt);
    }
    if (m_settings_panel && m_settings_panel->is_open()) {
        m_settings_panel->update(dt);
        
        // Check if window needs to be recreated
        if (m_settings_panel->needs_window_recreate()) {
            sf::Vector2u new_size;
            bool fullscreen;
            m_settings_panel->get_new_window_settings(new_size, fullscreen);
            m_settings_panel->clear_window_recreate_flag();
            
            // Recreate window with new settings
            if (fullscreen) {
                m_window.create(sf::VideoMode(new_size.x, new_size.y), "R-TYPE - Multiplayer", sf::Style::Fullscreen);
            } else {
                m_window.create(sf::VideoMode(new_size.x, new_size.y), "R-TYPE - Multiplayer");
            }
            m_window.setVerticalSyncEnabled(false);
            m_window.setFramerateLimit(60);
            
            // Recreate UI elements with new window size
            m_buttons.clear();
            m_corners.clear();
            m_side_panels.clear();
            m_background.reset();
            m_title.reset();
            m_footer.reset();
            m_settings_panel.reset();
            setup_ui();
        }
    }
}

void MenuState::render(sf::RenderWindow& window) {
    if (m_background) {
        m_background->render(window);
    }

    for (auto& panel : m_side_panels) {
        panel->render(window);
    }

    for (auto& corner : m_corners) {
        corner->render(window);
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
    if (m_settings_panel && m_settings_panel->is_open()) {
        m_settings_panel->render(window);
    }
}

}  // namespace rtype
