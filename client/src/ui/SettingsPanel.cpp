#include "ui/SettingsPanel.hpp"

#include "common/Settings.hpp"
#include "managers/AudioManager.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <iostream>

using namespace rtype::ui;

static std::string key_to_string(sf::Keyboard::Key k) {
    int ki = static_cast<int>(k);
    int a = static_cast<int>(sf::Keyboard::A);
    int z = static_cast<int>(sf::Keyboard::Z);
    if (ki >= a && ki <= z) {
        char c = static_cast<char>('A' + (ki - a));
        return std::string(1, c);
    }
    int n0 = static_cast<int>(sf::Keyboard::Num0);
    int n9 = static_cast<int>(sf::Keyboard::Num9);
    if (ki >= n0 && ki <= n9) {
        char c = static_cast<char>('0' + (ki - n0));
        return std::string(1, c);
    }
    int np0 = static_cast<int>(sf::Keyboard::Numpad0);
    int np9 = static_cast<int>(sf::Keyboard::Numpad9);
    if (ki >= np0 && ki <= np9) {
        return std::string("Numpad") + std::to_string(ki - np0);
    }

    switch (k) {
        case sf::Keyboard::Space:
            return "Space";
        case sf::Keyboard::Escape:
            return "Escape";
        case sf::Keyboard::Enter:
            return "Enter";
        case sf::Keyboard::BackSpace:
            return "Backspace";
        case sf::Keyboard::Tab:
            return "Tab";
        case sf::Keyboard::PageUp:
            return "PageUp";
        case sf::Keyboard::PageDown:
            return "PageDown";
        case sf::Keyboard::End:
            return "End";
        case sf::Keyboard::Home:
            return "Home";
        case sf::Keyboard::Insert:
            return "Insert";
        case sf::Keyboard::Delete:
            return "Delete";
        case sf::Keyboard::Add:
            return "+";
        case sf::Keyboard::Subtract:
            return "-";
        case sf::Keyboard::Multiply:
            return "*";
        case sf::Keyboard::Divide:
            return "/";
        case sf::Keyboard::Left:
            return "Left";
        case sf::Keyboard::Right:
            return "Right";
        case sf::Keyboard::Up:
            return "Up";
        case sf::Keyboard::Down:
            return "Down";
        case sf::Keyboard::LShift:
            return "LShift";
        case sf::Keyboard::RShift:
            return "RShift";
        case sf::Keyboard::LControl:
            return "LControl";
        case sf::Keyboard::RControl:
            return "RControl";
        case sf::Keyboard::LAlt:
            return "LAlt";
        case sf::Keyboard::RAlt:
            return "RAlt";
        case sf::Keyboard::SemiColon:
            return ";";
        case sf::Keyboard::Comma:
            return ",";
        case sf::Keyboard::Period:
            return ".";
        case sf::Keyboard::Quote:
            return "'";
        case sf::Keyboard::Slash:
            return "/";
        case sf::Keyboard::BackSlash:
            return "\\";
        case sf::Keyboard::Tilde:
            return "~";
        case sf::Keyboard::Equal:
            return "=";
        case sf::Keyboard::Hyphen:
            return "-";
        default:
            return std::string("Key(") + std::to_string(ki) + ")";
    }
}

SettingsPanel::SettingsPanel(const sf::Vector2u& window_size) : m_window_size(window_size) {
    m_overlay.setSize(
        sf::Vector2f(static_cast<float>(window_size.x), static_cast<float>(window_size.y)));
    m_overlay.setFillColor(sf::Color(0, 0, 0, 160));
    m_overlay.setPosition(0.0f, 0.0f);

    m_panel_bg.setSize(sf::Vector2f(900.0f, 600.0f));
    m_panel_bg.setFillColor(sf::Color(15, 15, 20, 240));
    m_panel_bg.setOutlineThickness(3.0f);
    m_panel_bg.setOutlineColor(sf::Color(100, 180, 220, 180));
    m_panel_bg.setPosition((static_cast<float>(window_size.x) - m_panel_bg.getSize().x) / 2.0f,
                           (static_cast<float>(window_size.y) - m_panel_bg.getSize().y) / 2.0f);

    if (m_font.loadFromFile("assets/fonts/arial.ttf")) {
        m_title.setFont(m_font);
    }
    m_title.setString("Settings");
    m_title.setCharacterSize(42);
    m_title.setFillColor(sf::Color(100, 200, 255));
    m_title.setPosition(m_panel_bg.getPosition().x + 24.0f, m_panel_bg.getPosition().y + 20.0f);

    m_players_text.setFont(m_font);
    m_players_text.setCharacterSize(24);
    m_players_text.setFillColor(sf::Color(100, 200, 255));
    m_players_text.setStyle(sf::Text::Bold);
    m_players_text.setPosition(m_panel_bg.getPosition().x + 24.0f,
                               m_panel_bg.getPosition().y + 160.0f);
    m_players_text.setString("Joueurs connectes: 0");

    float content_x = m_panel_bg.getPosition().x + 24.0f;
    float content_y = m_panel_bg.getPosition().y + 160.0f;

    m_volume_text.setFont(m_font);
    m_volume_text.setCharacterSize(22);
    m_volume_text.setFillColor(sf::Color(220, 220, 220));
    m_volume_text.setPosition(content_x, content_y);
    m_volume_text.setString("Volume: " + std::to_string(Settings::instance().master_volume) + "%");

    m_volume_bar.setSize(sf::Vector2f(300.0f, 32.0f));
    m_volume_bar.setFillColor(sf::Color(40, 40, 50));
    m_volume_bar.setOutlineThickness(2.0f);
    m_volume_bar.setOutlineColor(sf::Color(80, 80, 100));
    m_volume_bar.setPosition(content_x, content_y + 40.0f);

    m_music_volume_text.setFont(m_font);
    m_music_volume_text.setCharacterSize(22);
    m_music_volume_text.setFillColor(sf::Color(220, 220, 220));
    m_music_volume_text.setPosition(content_x, content_y);
    m_music_volume_text.setString("Music: " + std::to_string(Settings::instance().music_volume) +
                                  "%");

    m_music_volume_bar.setSize(sf::Vector2f(300.0f, 32.0f));
    m_music_volume_bar.setFillColor(sf::Color(40, 40, 50));
    m_music_volume_bar.setOutlineThickness(2.0f);
    m_music_volume_bar.setOutlineColor(sf::Color(80, 80, 100));
    m_music_volume_bar.setPosition(content_x, content_y + 40.0f);

    m_effects_volume_text.setFont(m_font);
    m_effects_volume_text.setCharacterSize(22);
    m_effects_volume_text.setFillColor(sf::Color(220, 220, 220));
    m_effects_volume_text.setPosition(content_x, content_y);
    m_effects_volume_text.setString(
        "Effects: " + std::to_string(Settings::instance().effects_volume) + "%");

    m_effects_volume_bar.setSize(sf::Vector2f(300.0f, 32.0f));
    m_effects_volume_bar.setFillColor(sf::Color(40, 40, 50));
    m_effects_volume_bar.setOutlineThickness(2.0f);
    m_effects_volume_bar.setOutlineColor(sf::Color(80, 80, 100));
    m_effects_volume_bar.setPosition(content_x, content_y + 40.0f);

    m_resolution_text.setFont(m_font);
    m_resolution_text.setCharacterSize(22);
    m_resolution_text.setFillColor(sf::Color(220, 220, 220));
    auto res = Settings::instance().resolutions[Settings::instance().resolution_index];
    m_resolution_text.setString("Resolution: " + std::to_string(res.first) + "x" +
                                std::to_string(res.second));
    m_resolution_text.setPosition(content_x, content_y);

    m_controls_text.setFont(m_font);
    m_controls_text.setCharacterSize(20);
    m_controls_text.setFillColor(sf::Color(200, 200, 200));
    m_controls_text.setPosition(content_x, content_y);
    m_controls_text.setString("");

    create_buttons();

    auto& audio = managers::AudioManager::instance();
    auto& s = Settings::instance();
    audio.set_master_volume(static_cast<float>(s.master_volume));
    audio.set_music_volume(static_cast<float>(s.music_volume));
    audio.set_sound_volume(static_cast<float>(s.effects_volume));
    audio.set_music_muted(s.music_muted);
    audio.set_sound_muted(s.effects_muted);
}

void SettingsPanel::create_buttons() {
    m_buttons.clear();
    m_tab_buttons.clear();
    m_tab_indicators.clear();

    float tab_x = m_panel_bg.getPosition().x + 24.0f;
    float tab_y = m_panel_bg.getPosition().y + 80.0f;
    float tab_width = 150.0f;
    float tab_spacing = 10.0f;

    std::vector<std::string> tab_labels;
    int base_index = 0;

    if (m_in_game_mode) {
        tab_labels = {"Game", "Audio", "Video", "Controls"};
        base_index = 0;
    } else {
        tab_labels = {"Audio", "Video", "Controls"};
        base_index = 1;
        if (static_cast<int>(m_current_tab) == 0) {
            m_current_tab = Tab::Audio;
        }
    }

    for (size_t i = 0; i < tab_labels.size(); ++i) {
        auto tab_btn = std::make_unique<Button>(
            sf::Vector2f(tab_x + static_cast<float>(i) * (tab_width + tab_spacing), tab_y),
            sf::Vector2f(tab_width, 45.0f), tab_labels[i]);

        Tab target_tab = static_cast<Tab>(base_index + static_cast<int>(i));
        tab_btn->set_callback([this, target_tab]() { this->switch_tab(target_tab); });

        if (tab_labels[i] == "Game") {
            tab_btn->set_colors(sf::Color(40, 160, 220, 220), sf::Color(70, 200, 255, 255),
                                sf::Color(20, 100, 160, 255));
        } else if (tab_labels[i] == "Audio") {
            tab_btn->set_colors(sf::Color(180, 80, 200, 220), sf::Color(210, 120, 240, 255),
                                sf::Color(140, 50, 160, 255));
        } else if (tab_labels[i] == "Video") {
            tab_btn->set_colors(sf::Color(200, 160, 40, 220), sf::Color(240, 200, 80, 255),
                                sf::Color(160, 120, 20, 255));
        } else {
            tab_btn->set_colors(sf::Color(80, 200, 120, 220), sf::Color(120, 240, 160, 255),
                                sf::Color(50, 160, 80, 255));
        }

        tab_btn->update(0.0f);
        m_tab_buttons.push_back(std::move(tab_btn));

        sf::RectangleShape indicator;
        indicator.setSize(sf::Vector2f(tab_width, 4.0f));
        indicator.setFillColor(sf::Color(100, 200, 255));
        indicator.setPosition(tab_x + static_cast<float>(i) * (tab_width + tab_spacing),
                              tab_y + 45.0f);
        m_tab_indicators.push_back(indicator);
    }

    float bx = m_panel_bg.getPosition().x + 24.0f;
    float by = m_panel_bg.getPosition().y + 250.0f;

    if (m_current_tab == Tab::Game) {
        float center_x = m_panel_bg.getPosition().x + (m_panel_bg.getSize().x - 220.0f) / 2.0f;
        float center_y = m_panel_bg.getPosition().y + (m_panel_bg.getSize().y - 100.0f) / 2.0f;

        auto quit_btn = std::make_unique<Button>(sf::Vector2f(center_x, center_y),
                                                 sf::Vector2f(220.0f, 50.0f), "Quit Game");
        quit_btn->set_callback([this]() {
            if (m_quit_callback)
                m_quit_callback();
        });
        quit_btn->set_colors(sf::Color(180, 50, 60, 255), sf::Color(220, 70, 80, 255),
                             sf::Color(140, 30, 40, 255));
        quit_btn->update(0.0f);
        m_buttons.push_back(std::move(quit_btn));
    }

    if (m_current_tab == Tab::Audio) {
        float base_y = m_panel_bg.getPosition().y + 140.0f;

        // Music mute toggle
        float music_mute_y = base_y + 130.0f + 40.0f;
        auto music_toggle = std::make_unique<Button>(
            sf::Vector2f(bx + 320.0f, music_mute_y), sf::Vector2f(140.0f, 45.0f),
            managers::AudioManager::instance().is_music_muted() ? "Unmute" : "Mute");
        music_toggle->set_callback([this]() {
            auto& audio = managers::AudioManager::instance();
            audio.set_music_muted(!audio.is_music_muted());
            auto& s = Settings::instance();
            s.music_muted = audio.is_music_muted();
            create_buttons();
        });
        if (managers::AudioManager::instance().is_music_muted()) {
            music_toggle->set_colors(sf::Color(150, 30, 30, 220), sf::Color(200, 50, 50, 255),
                                     sf::Color(120, 20, 20, 255));
        } else {
            music_toggle->set_colors(sf::Color(30, 150, 80, 220), sf::Color(50, 200, 120, 255),
                                     sf::Color(20, 120, 60, 255));
        }
        music_toggle->update(0.0f);
        m_buttons.push_back(std::move(music_toggle));

        // Sound effects mute toggle
        float effects_mute_y = music_mute_y + 130.0f;
        auto sound_toggle = std::make_unique<Button>(
            sf::Vector2f(bx + 320.0f, effects_mute_y), sf::Vector2f(140.0f, 45.0f),
            managers::AudioManager::instance().is_sound_muted() ? "Unmute" : "Mute");
        sound_toggle->set_callback([this]() {
            auto& audio = managers::AudioManager::instance();
            audio.set_sound_muted(!audio.is_sound_muted());
            auto& s = Settings::instance();
            s.effects_muted = audio.is_sound_muted();
            create_buttons();
        });
        if (managers::AudioManager::instance().is_sound_muted()) {
            sound_toggle->set_colors(sf::Color(150, 30, 30, 220), sf::Color(200, 50, 50, 255),
                                     sf::Color(120, 20, 20, 255));
        } else {
            sound_toggle->set_colors(sf::Color(30, 150, 80, 220), sf::Color(50, 200, 120, 255),
                                     sf::Color(20, 120, 60, 255));
        }
        sound_toggle->update(0.0f);
        m_buttons.push_back(std::move(sound_toggle));

    } else if (m_current_tab == Tab::Video) {
        auto res_prev =
            std::make_unique<Button>(sf::Vector2f(bx, by), sf::Vector2f(100.0f, 45.0f), "<");
        res_prev->set_callback([this]() {
            if (m_temp_resolution_index > 0)
                m_temp_resolution_index--;
        });
        res_prev->set_colors(sf::Color(80, 80, 100, 220), sf::Color(120, 120, 150, 255),
                             sf::Color(60, 60, 80, 255));
        res_prev->update(0.0f);
        m_buttons.push_back(std::move(res_prev));

        auto res_next = std::make_unique<Button>(sf::Vector2f(bx + 110.0f, by),
                                                 sf::Vector2f(100.0f, 45.0f), ">");
        res_next->set_callback([this]() {
            if (m_temp_resolution_index + 1 < Settings::instance().resolutions.size())
                ++m_temp_resolution_index;
        });
        res_next->set_colors(sf::Color(80, 80, 100, 220), sf::Color(120, 120, 150, 255),
                             sf::Color(60, 60, 80, 255));
        res_next->update(0.0f);
        m_buttons.push_back(std::move(res_next));

        float fs_y = by + 80.0f;
        auto fullscreen_btn = std::make_unique<Button>(
            sf::Vector2f(bx, fs_y), sf::Vector2f(220.0f, 45.0f), "Toggle Fullscreen");
        fullscreen_btn->set_callback([this]() {
            m_temp_fullscreen = !m_temp_fullscreen;
            create_buttons();
        });
        if (m_temp_fullscreen) {
            fullscreen_btn->set_colors(sf::Color(30, 150, 80, 220), sf::Color(50, 200, 120, 255),
                                       sf::Color(20, 120, 60, 255));
        } else {
            fullscreen_btn->set_colors(sf::Color(80, 80, 100, 220), sf::Color(120, 120, 150, 255),
                                       sf::Color(60, 60, 80, 255));
        }
        fullscreen_btn->update(0.0f);
        m_buttons.push_back(std::move(fullscreen_btn));

        float cb_y = by + 145.0f;

        // Helper pour obtenir le nom du mode
        auto get_mode_name = [](ColorBlindMode mode) -> std::string {
            switch (mode) {
                case ColorBlindMode::Normal:
                    return "Mode: Normal";
                case ColorBlindMode::Protanopia:
                    return "Mode: Protanopia";
                case ColorBlindMode::Deuteranopia:
                    return "Mode: Deuteranopia";
                case ColorBlindMode::Tritanopia:
                    return "Mode: Tritanopia";
                case ColorBlindMode::HighContrast:
                    return "Mode: Contraste++";
                default:
                    return "Mode: Normal";
            }
        };

        auto colorblind_btn = std::make_unique<Button>(
            sf::Vector2f(bx, cb_y), sf::Vector2f(250.0f, 45.0f), get_mode_name(m_temp_colorblind));
        colorblind_btn->set_callback([this]() {
            // Cycle entre les modes à chaque clic
            int current = static_cast<int>(m_temp_colorblind);
            current = (current + 1) % 5;  // 5 modes au total
            m_temp_colorblind = static_cast<ColorBlindMode>(current);

            // Appliquer immédiatement pour voir le changement en direct
            Settings::instance().colorblind_mode = m_temp_colorblind;

            create_buttons();
        });

        // Couleur différente selon le mode actif
        if (m_temp_colorblind != ColorBlindMode::Normal) {
            colorblind_btn->set_colors(sf::Color(200, 120, 30, 220), sf::Color(230, 160, 50, 255),
                                       sf::Color(160, 90, 20, 255));
        } else {
            colorblind_btn->set_colors(sf::Color(80, 80, 100, 220), sf::Color(120, 120, 150, 255),
                                       sf::Color(60, 60, 80, 255));
        }
        colorblind_btn->update(0.0f);
        m_buttons.push_back(std::move(colorblind_btn));

        float ss_y = cb_y + 60.0f;
        auto shake_btn = std::make_unique<Button>(sf::Vector2f(bx, ss_y),
                                                  sf::Vector2f(250.0f, 45.0f), "Screen Shake");
        shake_btn->set_callback([this]() {
            m_temp_screen_shake = !m_temp_screen_shake;
            create_buttons();
        });
        if (m_temp_screen_shake) {
            shake_btn->set_colors(sf::Color(200, 120, 30, 220), sf::Color(230, 160, 50, 255),
                                  sf::Color(160, 90, 20, 255));
        } else {
            shake_btn->set_colors(sf::Color(80, 80, 100, 220), sf::Color(120, 120, 150, 255),
                                  sf::Color(60, 60, 80, 255));
        }
        shake_btn->update(0.0f);
        m_buttons.push_back(std::move(shake_btn));

        sf::FloatRect bounds = m_resolution_text.getLocalBounds();
        m_resolution_text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        m_resolution_text.setPosition(bx + 105.0f, by - 25.0f);
    }

    else if (m_current_tab == Tab::Controls) {
        auto& s = Settings::instance();
        float left_x = bx;
        float right_x = bx + 260.0f;
        float cy_left = by;
        float cy_right = by;
        auto make_btn_at = [&](const std::string& label, int keycode, int action_index, float px,
                               float py) {
            std::string text;
            if (m_listening_control == action_index)
                text = "Press a key...";
            else
                text = label + ": " + key_to_string(static_cast<sf::Keyboard::Key>(keycode));

            auto btn =
                std::make_unique<Button>(sf::Vector2f(px, py), sf::Vector2f(220.0f, 45.0f), text);
            btn->set_callback([this, action_index]() {
                m_listening_control = action_index;
                create_buttons();
            });
            btn->set_colors(sf::Color(80, 80, 100, 220), sf::Color(120, 120, 150, 255),
                            sf::Color(60, 60, 80, 255));
            btn->update(0.0f);
            m_buttons.push_back(std::move(btn));
        };

        make_btn_at("Up", s.key_up, static_cast<int>(ControlAction::Up), left_x, cy_left);
        cy_left += 60.0f;
        make_btn_at("Down", s.key_down, static_cast<int>(ControlAction::Down), left_x, cy_left);
        cy_left += 60.0f;
        make_btn_at("Left", s.key_left, static_cast<int>(ControlAction::Left), left_x, cy_left);
        cy_left += 60.0f;
        make_btn_at("Right", s.key_right, static_cast<int>(ControlAction::Right), left_x, cy_left);

        make_btn_at("Shoot", s.key_shoot, static_cast<int>(ControlAction::Shoot), right_x,
                    cy_right);
        cy_right += 60.0f;
        make_btn_at("Power1", s.key_powerup1, static_cast<int>(ControlAction::Power1), right_x,
                    cy_right);
        cy_right += 60.0f;
        make_btn_at("Power2", s.key_powerup2, static_cast<int>(ControlAction::Power2), right_x,
                    cy_right);
        cy_right += 60.0f;

        // Auto Fire toggle
        auto autofire_btn = std::make_unique<Button>(sf::Vector2f(right_x, cy_right),
                                                     sf::Vector2f(220.0f, 45.0f), "Auto Fire");
        autofire_btn->set_callback([this]() {
            m_temp_auto_fire = !m_temp_auto_fire;
            create_buttons();
        });
        if (m_temp_auto_fire) {
            autofire_btn->set_colors(sf::Color(200, 120, 30, 220), sf::Color(230, 160, 50, 255),
                                     sf::Color(160, 90, 20, 255));
        } else {
            autofire_btn->set_colors(sf::Color(80, 80, 100, 220), sf::Color(120, 120, 150, 255),
                                     sf::Color(60, 60, 80, 255));
        }
        autofire_btn->update(0.0f);
        m_buttons.push_back(std::move(autofire_btn));
    }

    float bottom_y = m_panel_bg.getPosition().y + m_panel_bg.getSize().y - 70.0f;

    auto apply_btn =
        std::make_unique<Button>(sf::Vector2f(m_panel_bg.getPosition().x + 24.0f, bottom_y),
                                 sf::Vector2f(180.0f, 50.0f), "Apply");
    apply_btn->set_callback([this]() { this->apply_settings(); });
    apply_btn->set_colors(sf::Color(30, 150, 80, 220), sf::Color(50, 200, 120, 255),
                          sf::Color(20, 120, 60, 255));
    apply_btn->update(0.0f);
    m_buttons.push_back(std::move(apply_btn));

    auto back_btn = std::make_unique<Button>(
        sf::Vector2f(m_panel_bg.getPosition().x + m_panel_bg.getSize().x - 204.0f, bottom_y),
        sf::Vector2f(180.0f, 50.0f), "Close");
    back_btn->set_callback([this]() { this->close(); });
    back_btn->set_colors(sf::Color(180, 50, 60, 220), sf::Color(220, 70, 80, 255),
                         sf::Color(140, 30, 40, 255));
    back_btn->update(0.0f);
    m_buttons.push_back(std::move(back_btn));
}

void SettingsPanel::switch_tab(Tab new_tab) {
    m_current_tab = new_tab;
    m_selected_button = 0;
    create_buttons();
}

void SettingsPanel::apply_settings() {
    auto& s = Settings::instance();
    bool changed =
        (m_temp_resolution_index != s.resolution_index) || (m_temp_fullscreen != s.fullscreen);

    s.resolution_index = m_temp_resolution_index;
    s.fullscreen = m_temp_fullscreen;
    s.colorblind_mode = m_temp_colorblind;
    s.screen_shake_enabled = m_temp_screen_shake;
    s.auto_fire_enabled = m_temp_auto_fire;

    if (changed) {
        m_needs_window_recreate = true;
    }
}

void SettingsPanel::get_new_window_settings(sf::Vector2u& size, bool& fullscreen) const {
    auto& s = Settings::instance();
    auto res = s.resolutions[s.resolution_index];
    size =
        sf::Vector2u(static_cast<unsigned int>(res.first), static_cast<unsigned int>(res.second));
    fullscreen = s.fullscreen;
}

void SettingsPanel::handle_key_press(sf::Keyboard::Key key) {
    // Si on est en train d'écouter pour une touche de contrôle
    if (m_listening_control >= 0) {
        auto& s = Settings::instance();
        int code = static_cast<int>(key);
        switch (static_cast<ControlAction>(m_listening_control)) {
            case ControlAction::Up:
                s.key_up = code;
                break;
            case ControlAction::Down:
                s.key_down = code;
                break;
            case ControlAction::Left:
                s.key_left = code;
                break;
            case ControlAction::Right:
                s.key_right = code;
                break;
            case ControlAction::Shoot:
                s.key_shoot = code;
                break;
            case ControlAction::Power1:
                s.key_powerup1 = code;
                break;
            case ControlAction::Power2:
                s.key_powerup2 = code;
                break;
        }
        m_listening_control = -1;
        create_buttons();
        return;
    }

    // Navigation : gauche/droite pour les onglets, haut/bas pour les boutons, Entrée pour activer
    if (key == sf::Keyboard::Left) {
        // Changer d'onglet vers la gauche
        int current = static_cast<int>(m_current_tab);
        int min_index = m_in_game_mode ? 0 : 1;
        if (current > min_index) {
            switch_tab(static_cast<Tab>(current - 1));
        }
    } else if (key == sf::Keyboard::Right) {
        // Changer d'onglet vers la droite
        int current = static_cast<int>(m_current_tab);
        int max_index = 3;  // Controls est toujours à l'index 3
        if (current < max_index) {
            switch_tab(static_cast<Tab>(current + 1));
        }
    } else if (key == sf::Keyboard::Up) {
        m_keyboard_navigation = true;
        if (m_selected_button > 0) {
            m_selected_button--;
            managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Plop);
        }
    } else if (key == sf::Keyboard::Down) {
        m_keyboard_navigation = true;
        if (m_selected_button + 1 < m_buttons.size()) {
            m_selected_button++;
            managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Plop);
        }
    } else if (key == sf::Keyboard::Return || key == sf::Keyboard::Space) {
        // Activer le bouton sélectionné
        if (m_selected_button < m_buttons.size()) {
            m_buttons[m_selected_button]->trigger();
        }
    }
}

void SettingsPanel::handle_mouse_move(const sf::Vector2f& mouse_pos) {
    // Désactiver la navigation clavier dès que la souris bouge
    m_keyboard_navigation = false;

    // Handle volume bar dragging
    if (m_dragging_bar != DraggingBar::None && m_current_tab == Tab::Audio) {
        auto& s = Settings::instance();
        auto& audio = managers::AudioManager::instance();

        if (m_dragging_bar == DraggingBar::Master &&
            m_volume_bar.getGlobalBounds().contains(mouse_pos)) {
            float x_offset = mouse_pos.x - m_volume_bar.getPosition().x;
            float percentage = std::clamp(x_offset / m_volume_bar.getSize().x, 0.0f, 1.0f);
            s.master_volume = static_cast<int>(percentage * 100.0f);
            audio.set_master_volume(static_cast<float>(s.master_volume));
        } else if (m_dragging_bar == DraggingBar::Music &&
                   m_music_volume_bar.getGlobalBounds().contains(mouse_pos)) {
            float x_offset = mouse_pos.x - m_music_volume_bar.getPosition().x;
            float percentage = std::clamp(x_offset / m_music_volume_bar.getSize().x, 0.0f, 1.0f);
            s.music_volume = static_cast<int>(percentage * 100.0f);
            audio.set_music_volume(static_cast<float>(s.music_volume));
        } else if (m_dragging_bar == DraggingBar::Effects &&
                   m_effects_volume_bar.getGlobalBounds().contains(mouse_pos)) {
            float x_offset = mouse_pos.x - m_effects_volume_bar.getPosition().x;
            float percentage = std::clamp(x_offset / m_effects_volume_bar.getSize().x, 0.0f, 1.0f);
            s.effects_volume = static_cast<int>(percentage * 100.0f);
            audio.set_sound_volume(static_cast<float>(s.effects_volume));
        }
    }

    for (auto& b : m_tab_buttons)
        b->handle_mouse_move(mouse_pos);
    for (auto& b : m_buttons)
        b->handle_mouse_move(mouse_pos);
}

void SettingsPanel::handle_mouse_click(const sf::Vector2f& mouse_pos) {
    // Check if clicking on volume bars in Audio tab
    if (m_current_tab == Tab::Audio) {
        auto& s = Settings::instance();
        auto& audio = managers::AudioManager::instance();

        // Master volume bar
        if (m_volume_bar.getGlobalBounds().contains(mouse_pos)) {
            m_dragging_bar = DraggingBar::Master;
            float x_offset = mouse_pos.x - m_volume_bar.getPosition().x;
            float percentage = std::clamp(x_offset / m_volume_bar.getSize().x, 0.0f, 1.0f);
            s.master_volume = static_cast<int>(percentage * 100.0f);
            audio.set_master_volume(static_cast<float>(s.master_volume));
            return;
        }

        // Music volume bar
        if (m_music_volume_bar.getGlobalBounds().contains(mouse_pos)) {
            m_dragging_bar = DraggingBar::Music;
            float x_offset = mouse_pos.x - m_music_volume_bar.getPosition().x;
            float percentage = std::clamp(x_offset / m_music_volume_bar.getSize().x, 0.0f, 1.0f);
            s.music_volume = static_cast<int>(percentage * 100.0f);
            audio.set_music_volume(static_cast<float>(s.music_volume));
            return;
        }

        // Effects volume bar
        if (m_effects_volume_bar.getGlobalBounds().contains(mouse_pos)) {
            m_dragging_bar = DraggingBar::Effects;
            float x_offset = mouse_pos.x - m_effects_volume_bar.getPosition().x;
            float percentage = std::clamp(x_offset / m_effects_volume_bar.getSize().x, 0.0f, 1.0f);
            s.effects_volume = static_cast<int>(percentage * 100.0f);
            audio.set_sound_volume(static_cast<float>(s.effects_volume));
            return;
        }

        // If clicking elsewhere, stop dragging
        m_dragging_bar = DraggingBar::None;
    }

    for (auto& b : m_tab_buttons) {
        if (b->handle_mouse_click(mouse_pos))
            return;
    }
    for (auto& b : m_buttons) {
        if (b->handle_mouse_click(mouse_pos))
            break;
    }
}

void SettingsPanel::handle_mouse_release(const sf::Vector2f& mouse_pos) {
    (void)mouse_pos;  // Unused but keep signature consistent
    m_dragging_bar = DraggingBar::None;
}

void SettingsPanel::update(float dt) {
    m_volume_text.setString("Master: " + std::to_string(Settings::instance().master_volume) + "%");
    m_music_volume_text.setString("Music: " + std::to_string(Settings::instance().music_volume) +
                                  "%");
    m_effects_volume_text.setString(
        "Effects: " + std::to_string(Settings::instance().effects_volume) + "%");

    if (m_in_game_mode) {
        m_players_text.setString("Joueurs connectes: " + std::to_string(m_connected_players));
    }

    if (m_current_tab == Tab::Video) {
        auto res = Settings::instance().resolutions[m_temp_resolution_index];
        m_resolution_text.setString("Resolution: " + std::to_string(res.first) + "x" +
                                    std::to_string(res.second));

        sf::FloatRect bounds = m_resolution_text.getLocalBounds();
        m_resolution_text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);

        float bx = m_panel_bg.getPosition().x + 24.0f;
        float by = m_panel_bg.getPosition().y + 250.0f;
        m_resolution_text.setPosition(bx + 105.0f, by - 25.0f);
    }

    for (auto& b : m_tab_buttons)
        b->update(dt);

    // Mettre à jour tous les boutons
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        // Si on est en mode navigation clavier, gérer le highlight manuellement
        if (m_keyboard_navigation) {
            // Reset tous les boutons puis highlight uniquement le sélectionné
            m_buttons[i]->set_hovered(false);
            if (i == m_selected_button) {
                m_buttons[i]->set_hovered(true);
            }
        }
        m_buttons[i]->update(dt);
    }
}

void SettingsPanel::render(sf::RenderWindow& window) {
    if (!m_open)
        return;

    window.draw(m_overlay);
    window.draw(m_panel_bg);
    window.draw(m_title);

    for (auto& b : m_tab_buttons)
        b->render(window);

    int indicator_index = static_cast<int>(m_current_tab) - (m_in_game_mode ? 0 : 1);
    if (indicator_index >= 0 && indicator_index < static_cast<int>(m_tab_indicators.size())) {
        window.draw(m_tab_indicators[static_cast<std::size_t>(indicator_index)]);
    }

    if (m_current_tab == Tab::Game) {
        window.draw(m_players_text);
    }

    if (m_current_tab == Tab::Audio) {
        float base_y = m_panel_bg.getPosition().y + 140.0f;
        float bx = m_panel_bg.getPosition().x + 24.0f;

        // Master volume
        m_volume_text.setPosition(bx, base_y);
        m_volume_bar.setPosition(bx, base_y + 40.0f);
        window.draw(m_volume_text);
        window.draw(m_volume_bar);
        float pct = static_cast<float>(Settings::instance().master_volume) / 100.0f;
        sf::RectangleShape inner(
            sf::Vector2f(m_volume_bar.getSize().x * pct, m_volume_bar.getSize().y));
        inner.setPosition(m_volume_bar.getPosition());
        inner.setFillColor(sf::Color(100, 180, 220));
        window.draw(inner);

        // Music volume
        float music_y = base_y + 130.0f;
        m_music_volume_text.setPosition(bx, music_y);
        m_music_volume_bar.setPosition(bx, music_y + 40.0f);
        window.draw(m_music_volume_text);
        window.draw(m_music_volume_bar);
        float music_pct = static_cast<float>(Settings::instance().music_volume) / 100.0f;
        sf::RectangleShape music_inner(sf::Vector2f(m_music_volume_bar.getSize().x * music_pct,
                                                    m_music_volume_bar.getSize().y));
        music_inner.setPosition(m_music_volume_bar.getPosition());
        music_inner.setFillColor(sf::Color(180, 100, 220));
        window.draw(music_inner);

        // Effects volume
        float effects_y = music_y + 130.0f;
        m_effects_volume_text.setPosition(bx, effects_y);
        m_effects_volume_bar.setPosition(bx, effects_y + 40.0f);
        window.draw(m_effects_volume_text);
        window.draw(m_effects_volume_bar);
        float effects_pct = static_cast<float>(Settings::instance().effects_volume) / 100.0f;
        sf::RectangleShape effects_inner(sf::Vector2f(
            m_effects_volume_bar.getSize().x * effects_pct, m_effects_volume_bar.getSize().y));
        effects_inner.setPosition(m_effects_volume_bar.getPosition());
        effects_inner.setFillColor(sf::Color(220, 180, 100));
        window.draw(effects_inner);
    }

    if (m_current_tab == Tab::Video) {
        window.draw(m_resolution_text);
    }

    if (m_current_tab == Tab::Controls) {}

    for (auto& b : m_buttons)
        b->render(window);
}