#include "states/SettingsState.hpp"
#include "core/SettingsManager.hpp"
#include <iostream>

namespace rtype {

SettingsState::SettingsState(sf::RenderWindow& window)
    : m_window(window)
    , m_next_state("")
    , m_fullscreen(false)
    , m_vsync(true)
    , m_show_fps(false)
    , m_resolution_index(0)
    , m_music_volume(70)
    , m_sfx_volume(80)
    , m_quality_index(2)
    , m_window_settings_changed(false)
{
    m_available_resolutions = {
        sf::VideoMode(1920, 1080),
        sf::VideoMode(1600, 900),
        sf::VideoMode(1280, 720),
        sf::VideoMode(1024, 768)
    };
    sf::Vector2u current = m_window.getSize();
    for (size_t i = 0; i < m_available_resolutions.size(); ++i) {
        if (m_available_resolutions[i].width == current.x &&
            m_available_resolutions[i].height == current.y) {
            m_resolution_index = static_cast<int>(i);
            break;
        }
    }
}

void SettingsState::on_enter() {
    std::cout << "[SettingsState] Entering settings\n";
    auto& settings = SettingsManager::get_instance();
    auto mode = settings.get_resolution();
    m_fullscreen = settings.is_fullscreen();
    m_vsync = settings.is_vsync_enabled();
    m_show_fps = settings.should_show_fps();
    m_music_volume = settings.get_music_volume();
    m_sfx_volume = settings.get_sfx_volume();
    m_quality_index = settings.get_graphics_quality();
    for (size_t i = 0; i < m_available_resolutions.size(); ++i) {
        if (m_available_resolutions[i].width == mode.width &&
            m_available_resolutions[i].height == mode.height) {
            m_resolution_index = static_cast<int>(i);
            break;
        }
    }
    setup_ui();
}

void SettingsState::on_exit() {
    std::cout << "[SettingsState] Exiting settings\n";
}

void SettingsState::setup_ui() {
    std::cout << "[SettingsState] setup_ui() called\n";
    sf::Vector2u window_size = m_window.getSize();
    std::cout << "[SettingsState] Window size: " << window_size.x << "x" << window_size.y << "\n";
    std::cout << "[SettingsState] Creating background...\n";
    m_background = std::make_unique<ui::MenuBackground>(window_size);
    std::cout << "[SettingsState] Background created\n";
    std::cout << "[SettingsState] Creating title...\n";
    sf::Vector2f title_pos(static_cast<float>(window_size.x) / 2.0f, 100.0f);
    m_title = std::make_unique<ui::MenuTitle>("SETTINGS", title_pos, 60);
    std::cout << "[SettingsState] Title created\n";
    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font for settings\n";
    }
    float panel_width = 900.0f;
    float panel_height = 600.0f;
    float panel_x = (static_cast<float>(window_size.x) - panel_width) / 2.0f;
    float panel_y = 180.0f;
    float line_height = 70.0f;
    std::vector<std::string> labels = {
        "Resolution:",
        "Fullscreen:",
        "VSync:",
        "Graphics Quality:",
        "Show FPS:",
        "Music Volume:",
        "SFX Volume:"
    };
    m_settings_labels.clear();
    m_settings_values.clear();
    for (size_t i = 0; i < labels.size(); ++i) {
        sf::Text label;
        label.setFont(m_font);
        label.setString(labels[i]);
        label.setCharacterSize(28);
        label.setFillColor(sf::Color(200, 200, 255));
        label.setPosition(panel_x + 50.0f, panel_y + static_cast<float>(i) * line_height);
        m_settings_labels.push_back(label);
        sf::Text value;
        value.setFont(m_font);
        value.setCharacterSize(28);
        value.setFillColor(sf::Color::White);
        value.setPosition(panel_x + panel_width - 350.0f, panel_y + static_cast<float>(i) * line_height);
        m_settings_values.push_back(value);
    }
    update_settings_display();
    m_buttons.clear();
    float button_y = panel_y;
    float button_x_left = panel_x + panel_width - 200.0f;
    float button_x_right = panel_x + panel_width - 110.0f;
    float small_button_width = 60.0f;
    float small_button_height = 32.0f;
    float toggle_button_width = 130.0f;
    auto res_left = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_left, button_y + 5.0f),
        sf::Vector2f(small_button_width, small_button_height), "<");
    res_left->set_colors(sf::Color(60, 60, 100, 200),
                         sf::Color(80, 80, 120, 255),
                         sf::Color(100, 150, 255));
    res_left->set_callback([this]() { change_resolution(-1); });
    m_buttons.push_back(std::move(res_left));
    auto res_right = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_right, button_y + 5.0f),
        sf::Vector2f(small_button_width, small_button_height), ">");
    res_right->set_colors(sf::Color(60, 60, 100, 200),
                          sf::Color(80, 80, 120, 255),
                          sf::Color(100, 150, 255));
    res_right->set_callback([this]() { change_resolution(1); });
    m_buttons.push_back(std::move(res_right));
    button_y += line_height;
    auto fullscreen_btn = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_left, button_y),
        sf::Vector2f(toggle_button_width, small_button_height), "TOGGLE");
    fullscreen_btn->set_colors(sf::Color(60, 60, 100, 200),
                               sf::Color(80, 80, 120, 255),
                               sf::Color(100, 150, 255));
    fullscreen_btn->set_callback([this]() { toggle_fullscreen(); });
    m_buttons.push_back(std::move(fullscreen_btn));
    button_y += line_height;
    auto vsync_btn = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_left, button_y),
        sf::Vector2f(toggle_button_width, small_button_height), "TOGGLE");
    vsync_btn->set_colors(sf::Color(60, 60, 100, 200),
                          sf::Color(80, 80, 120, 255),
                          sf::Color(100, 150, 255));
    vsync_btn->set_callback([this]() { toggle_vsync(); });
    m_buttons.push_back(std::move(vsync_btn));
    button_y += line_height;
    auto quality_left = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_left, button_y),
        sf::Vector2f(small_button_width, small_button_height), "<");
    quality_left->set_colors(sf::Color(60, 60, 100, 200),
                             sf::Color(80, 80, 120, 255),
                             sf::Color(100, 150, 255));
    quality_left->set_callback([this]() { change_quality(-1); });
    m_buttons.push_back(std::move(quality_left));
    auto quality_right = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_right, button_y),
        sf::Vector2f(small_button_width, small_button_height), ">");
    quality_right->set_colors(sf::Color(60, 60, 100, 200),
                              sf::Color(80, 80, 120, 255),
                              sf::Color(100, 150, 255));
    quality_right->set_callback([this]() { change_quality(1); });
    m_buttons.push_back(std::move(quality_right));
    button_y += line_height;
    auto fps_btn = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_left, button_y),
        sf::Vector2f(toggle_button_width, small_button_height), "TOGGLE");
    fps_btn->set_colors(sf::Color(60, 60, 100, 200),
                        sf::Color(80, 80, 120, 255),
                        sf::Color(100, 150, 255));
    fps_btn->set_callback([this]() { toggle_show_fps(); });
    m_buttons.push_back(std::move(fps_btn));
    button_y += line_height;
    auto music_left = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_left, button_y),
        sf::Vector2f(small_button_width, small_button_height), "-");
    music_left->set_colors(sf::Color(60, 60, 100, 200),
                           sf::Color(80, 80, 120, 255),
                           sf::Color(100, 150, 255));
    music_left->set_callback([this]() { change_volume("music", -10); });
    m_buttons.push_back(std::move(music_left));
    auto music_right = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_right, button_y),
        sf::Vector2f(small_button_width, small_button_height), "+");
    music_right->set_colors(sf::Color(60, 60, 100, 200),
                            sf::Color(80, 80, 120, 255),
                            sf::Color(100, 150, 255));
    music_right->set_callback([this]() { change_volume("music", 10); });
    m_buttons.push_back(std::move(music_right));
    button_y += line_height;
    auto sfx_left = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_left, button_y),
        sf::Vector2f(small_button_width, small_button_height), "-");
    sfx_left->set_colors(sf::Color(60, 60, 100, 200),
                         sf::Color(80, 80, 120, 255),
                         sf::Color(100, 150, 255));
    sfx_left->set_callback([this]() { change_volume("sfx", -10); });
    m_buttons.push_back(std::move(sfx_left));
    auto sfx_right = std::make_unique<ui::Button>(
        sf::Vector2f(button_x_right, button_y),
        sf::Vector2f(small_button_width, small_button_height), "+");
    sfx_right->set_colors(sf::Color(60, 60, 100, 200),
                          sf::Color(80, 80, 120, 255),
                          sf::Color(100, 150, 255));
    sfx_right->set_callback([this]() { change_volume("sfx", 10); });
    m_buttons.push_back(std::move(sfx_right));
    float bottom_button_y = panel_y + panel_height + 50.0f;
    float button_width = 180.0f;
    float button_height = 50.0f;
    float button_spacing = 30.0f;
    auto reset_btn = std::make_unique<ui::Button>(
        sf::Vector2f(static_cast<float>(window_size.x) / 2.0f - button_width * 1.5f - button_spacing, bottom_button_y),
        sf::Vector2f(button_width, button_height), "RESET");
    reset_btn->set_colors(sf::Color(80, 80, 50, 200),
                          sf::Color(100, 100, 70, 255),
                          sf::Color(150, 150, 100));
    reset_btn->set_callback([this]() { on_reset_defaults_clicked(); });
    m_buttons.push_back(std::move(reset_btn));
    auto apply_btn = std::make_unique<ui::Button>(
        sf::Vector2f(static_cast<float>(window_size.x) / 2.0f - button_width / 2.0f, bottom_button_y),
        sf::Vector2f(button_width, button_height), "APPLY");
    apply_btn->set_colors(sf::Color(50, 100, 50, 200),
                          sf::Color(70, 130, 70, 255),
                          sf::Color(100, 200, 100));
    apply_btn->set_callback([this]() { on_apply_clicked(); });
    m_buttons.push_back(std::move(apply_btn));
    auto back_btn = std::make_unique<ui::Button>(
        sf::Vector2f(static_cast<float>(window_size.x) / 2.0f + button_width / 2.0f + button_spacing, bottom_button_y),
        sf::Vector2f(button_width, button_height), "BACK");
    back_btn->set_colors(sf::Color(80, 50, 50, 200),
                         sf::Color(120, 70, 70, 255),
                         sf::Color(200, 100, 100));
    back_btn->set_callback([this]() { on_back_clicked(); });
    m_buttons.push_back(std::move(back_btn));
}

void SettingsState::update_settings_display() {
    if (m_settings_values.size() >= 7) {
        auto& res = m_available_resolutions[static_cast<std::size_t>(m_resolution_index)];
        m_settings_values[0].setString(std::to_string(res.width) + "x" + std::to_string(res.height));
        m_settings_values[1].setString(m_fullscreen ? "ON" : "OFF");
        m_settings_values[1].setFillColor(m_fullscreen ? sf::Color(100, 255, 100) : sf::Color::White);
        m_settings_values[2].setString(m_vsync ? "ON" : "OFF");
        m_settings_values[2].setFillColor(m_vsync ? sf::Color(100, 255, 100) : sf::Color::White);
        std::vector<std::string> quality_names = {"LOW", "MEDIUM", "HIGH", "ULTRA"};
        m_settings_values[3].setString(quality_names[static_cast<std::size_t>(m_quality_index)]);
        m_settings_values[4].setString(m_show_fps ? "ON" : "OFF");
        m_settings_values[4].setFillColor(m_show_fps ? sf::Color(100, 255, 100) : sf::Color::White);
        m_settings_values[5].setString(std::to_string(m_music_volume) + "%");
        m_settings_values[6].setString(std::to_string(m_sfx_volume) + "%");
    }
}

void SettingsState::toggle_fullscreen() {
    m_fullscreen = !m_fullscreen;
    m_window_settings_changed = true;
    update_settings_display();
    std::cout << "[SettingsState] Fullscreen toggled: " << (m_fullscreen ? "ON" : "OFF") << "\n";
}

void SettingsState::toggle_vsync() {
    m_vsync = !m_vsync;
    m_window.setVerticalSyncEnabled(m_vsync);
    update_settings_display();
    std::cout << "[SettingsState] VSync toggled: " << (m_vsync ? "ON" : "OFF") << " (applied immediately)\n";
}

void SettingsState::toggle_show_fps() {
    m_show_fps = !m_show_fps;
    update_settings_display();
    std::cout << "[SettingsState] Show FPS toggled: " << (m_show_fps ? "ON" : "OFF") << "\n";
}

void SettingsState::change_resolution(int direction) {
    m_resolution_index += direction;
    if (m_resolution_index < 0) {
        m_resolution_index = static_cast<int>(m_available_resolutions.size()) - 1;
    } else if (m_resolution_index >= static_cast<int>(m_available_resolutions.size())) {
        m_resolution_index = 0;
    }
    m_window_settings_changed = true;
    update_settings_display();
    auto& res = m_available_resolutions[static_cast<std::size_t>(m_resolution_index)];
    std::cout << "[SettingsState] Resolution changed to: " << res.width << "x" << res.height << "\n";
}

void SettingsState::change_quality(int direction) {
    m_quality_index += direction;
    if (m_quality_index < 0) {
        m_quality_index = 3; // Ultra
    } else if (m_quality_index > 3) {
        m_quality_index = 0; // Low
    }
    update_settings_display();
    std::vector<std::string> quality_names = {"Low", "Medium", "High", "Ultra"};
    std::cout << "[SettingsState] Graphics quality changed to: " << quality_names[static_cast<std::size_t>(m_quality_index)] << "\n";
}

void SettingsState::change_volume(const std::string& type, int direction) {
    if (type == "music") {
        m_music_volume += direction;
        if (m_music_volume < 0) m_music_volume = 0;
        if (m_music_volume > 100) m_music_volume = 100;
        std::cout << "[SettingsState] Music volume: " << m_music_volume << "%\n";
    } else if (type == "sfx") {
        m_sfx_volume += direction;
        if (m_sfx_volume < 0) m_sfx_volume = 0;
        if (m_sfx_volume > 100) m_sfx_volume = 100;
        std::cout << "[SettingsState] SFX volume: " << m_sfx_volume << "%\n";
    }
    update_settings_display();
}

void SettingsState::recreate_window_with_settings() {
    std::cout << "[SettingsState] Recreating window with new settings...\n";
    auto& res = m_available_resolutions[static_cast<std::size_t>(m_resolution_index)];
    sf::Uint32 style = m_fullscreen ? sf::Style::Fullscreen : sf::Style::Close;
    m_window.close();
    m_window.create(sf::VideoMode(res.width, res.height), "R-Type", style);
    m_window.setVerticalSyncEnabled(m_vsync);
    m_window.setFramerateLimit(60);
    std::cout << "[SettingsState] Window recreated: " << res.width << "x" << res.height 
              << " (Fullscreen: " << (m_fullscreen ? "ON" : "OFF") << ")\n";
    m_background.reset();
    m_title.reset();
    m_buttons.clear();
    setup_ui();
}

void SettingsState::on_apply_clicked() {
    std::cout << "[SettingsState] Applying settings...\n";
    auto& settings = SettingsManager::get_instance();
    auto& res = m_available_resolutions[static_cast<std::size_t>(m_resolution_index)];
    settings.set_resolution(res.width, res.height);
    settings.set_fullscreen(m_fullscreen);
    settings.set_vsync(m_vsync);
    settings.set_show_fps(m_show_fps);
    settings.set_graphics_quality(m_quality_index);
    settings.set_music_volume(m_music_volume);
    settings.set_sfx_volume(m_sfx_volume);
    settings.save_to_file("settings.ini");
    if (m_window_settings_changed) {
        recreate_window_with_settings();
        m_window_settings_changed = false;
    } else {
        apply_settings_immediately();
    }
    std::cout << "[SettingsState] Settings applied:" << std::endl;
    std::cout << "  - Resolution: " << res.width << "x" << res.height << " ✓" << std::endl;
    std::cout << "  - Fullscreen: " << (m_fullscreen ? "ON" : "OFF") << " ✓" << std::endl;
    std::cout << "  - VSync: " << (m_vsync ? "ON" : "OFF") << " ✓" << std::endl;
    std::cout << "  - Graphics: " << (m_quality_index == 0 ? "Low" : m_quality_index == 1 ? "Medium" : m_quality_index == 2 ? "High" : "Ultra") << std::endl;
    std::cout << "  - Show FPS: " << (m_show_fps ? "ON" : "OFF") << std::endl;
    std::cout << "  - Music: " << m_music_volume << "%" << std::endl;
    std::cout << "  - SFX: " << m_sfx_volume << "%" << std::endl;
    m_next_state = "menu";
}

void SettingsState::on_reset_defaults_clicked() {
    std::cout << "[SettingsState] Resetting to default settings...\n";
    auto& settings = SettingsManager::get_instance();
    settings.reset_to_defaults();
    m_fullscreen = settings.is_fullscreen();
    m_vsync = settings.is_vsync_enabled();
    m_show_fps = settings.should_show_fps();
    m_resolution_index = 0;
    m_music_volume = settings.get_music_volume();
    m_sfx_volume = settings.get_sfx_volume();
    m_quality_index = settings.get_graphics_quality();
    m_window_settings_changed = true;
    apply_settings_immediately();
    update_settings_display();
    std::cout << "[SettingsState] Settings reset to defaults\n";
}

void SettingsState::apply_settings_immediately() {
    m_window.setVerticalSyncEnabled(m_vsync);
    std::cout << "[SettingsState] Immediate settings applied (VSync: " << (m_vsync ? "ON" : "OFF") << ")\n";
}

void SettingsState::on_back_clicked() {
    std::cout << "[SettingsState] Going back to menu\n";
    m_next_state = "menu";
}

void SettingsState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        m_mouse_pos = sf::Vector2f(static_cast<float>(event.mouseMove.x),
                                    static_cast<float>(event.mouseMove.y));
        for (auto& button : m_buttons) {
            button->handle_mouse_move(m_mouse_pos);
        }
    }
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            for (auto& button : m_buttons) {
                button->handle_mouse_click(m_mouse_pos);
            }
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            on_back_clicked();
        }
    }
}

void SettingsState::update(float dt) {
    if (m_background) {
        m_background->update(dt);
    }
    if (m_title) {
        m_title->update(dt);
    }
    for (auto& button : m_buttons) {
        button->update(dt);
    }
}

void SettingsState::render(sf::RenderWindow& window) {
    if (m_background) {
        m_background->render(window);
    }
    if (m_title) {
        m_title->render(window);
    }
    sf::Vector2u window_size = window.getSize();
    float panel_width = 900.0f;
    float panel_height = 600.0f;
    float panel_x = (static_cast<float>(window_size.x) - panel_width) / 2.0f;
    float panel_y = 180.0f;
    sf::RectangleShape panel(sf::Vector2f(panel_width, panel_height));
    panel.setPosition(panel_x, panel_y);
    panel.setFillColor(sf::Color(20, 20, 40, 220));
    panel.setOutlineColor(sf::Color(100, 150, 255, 150));
    panel.setOutlineThickness(2.0f);
    window.draw(panel);
    for (const auto& label : m_settings_labels) {
        window.draw(label);
    }
    for (const auto& value : m_settings_values) {
        window.draw(value);
    }
    for (const auto& button : m_buttons) {
        button->render(window);
    }
}

} // namespace rtype
