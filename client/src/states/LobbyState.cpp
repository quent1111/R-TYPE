#include "states/LobbyState.hpp"

#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "common/Settings.hpp"
#include "level/CustomLevelLoader.hpp"
#include "managers/AudioManager.hpp"
#include "rendering/ColorBlindShader.hpp"

#include <filesystem>
#include <iostream>

namespace rtype {

LobbyState::LobbyState(sf::RenderWindow& window,
                       std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> game_to_net,
                       std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> net_to_game)
    : m_window(window),
      m_game_to_network_queue(game_to_net),
      m_network_to_game_queue(net_to_game) {}

LobbyState::~LobbyState() {
    on_exit();
}

void LobbyState::on_enter() {
    std::cout << "[LobbyState] Entering lobby\n";

    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[LobbyState] Warning: Could not load font\n";
    }

    m_buttons.clear();
    m_corners.clear();
    m_side_panels.clear();
    m_background.reset();
    m_title.reset();
    m_footer.reset();

    m_total_players = 1;
    m_ready_players = 0;

    m_available_levels = {"Standard Game"};
    m_available_level_ids = {""};

    std::filesystem::path custom_dir = "levels/custom";
    if (std::filesystem::exists(custom_dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(custom_dir)) {
            if (entry.path().extension() == ".json") {
                auto config = level::CustomLevelLoader::load_from_file(entry.path().string());
                if (config) {
                    m_available_levels.push_back(config->name);
                    m_available_level_ids.push_back(config->id);
                    std::cout << "[LobbyState] Found custom level: " << config->name
                              << " (id: " << config->id << ")" << std::endl;
                }
            }
        }
    }

    m_selected_level_index = 0;
    m_use_custom_level = false;

    setup_ui();

    request_lobby_status();
}

void LobbyState::on_exit() {
    std::cout << "[LobbyState] Exiting lobby\n";
}

void LobbyState::setup_ui() {
    auto window_size = m_window.getSize();
    sf::Vector2f center(static_cast<float>(window_size.x) / 2.0f,
                        static_cast<float>(window_size.y) / 2.0f);

    m_background = std::make_unique<ui::MenuBackground>(window_size);

    m_title = std::make_unique<ui::MenuTitle>("LOBBY", sf::Vector2f(center.x, 50.0f), 72);

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

    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(50.0f, static_cast<float>(window_size.y) / 2.0f - 50.0f), true));
    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(static_cast<float>(window_size.x) - 50.0f,
                     static_cast<float>(window_size.y) / 2.0f - 50.0f),
        false));

    m_info_text.setFont(m_font);
    m_info_text.setCharacterSize(20);
    m_info_text.setFillColor(sf::Color(150, 200, 255));
    m_info_text.setString("Waiting in lobby...");
    auto info_bounds = m_info_text.getLocalBounds();
    m_info_text.setPosition(center.x - info_bounds.width / 2.0f, 150.0f);

    m_player_list_text.setFont(m_font);
    m_player_list_text.setCharacterSize(16);
    m_player_list_text.setFillColor(sf::Color(200, 200, 200));
    m_player_list_text.setString("Players: 0");
    m_player_list_text.setPosition(center.x - 200.0f, 220.0f);

    m_status_text.setFont(m_font);
    m_status_text.setCharacterSize(40);
    m_status_text.setFillColor(sf::Color::White);
    m_status_text.setString("Joueurs: 0/4");
    auto status_bounds = m_status_text.getLocalBounds();
    m_status_text.setPosition(center.x - status_bounds.width / 2.0f, center.y - 100.0f);

    m_level_text.setFont(m_font);
    m_level_text.setCharacterSize(28);
    m_level_text.setFillColor(sf::Color(255, 255, 0));
    update_level_display();

    const float button_width = 300.0f;
    const float button_height = 60.0f;
    const float button_spacing = 70.0f;
    const float start_y = center.y + 50.0f;

    auto level_btn =
        std::make_unique<ui::Button>(sf::Vector2f(center.x - button_width / 2.0f, start_y),
                                     sf::Vector2f(button_width, button_height), "< SELECT LEVEL >");
    level_btn->set_colors(sf::Color(80, 80, 150, 220), sf::Color(100, 100, 200, 255),
                          sf::Color(60, 60, 120, 255));
    level_btn->set_callback([this]() { on_level_select_clicked(); });
    m_buttons.push_back(std::move(level_btn));

    auto start_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing),
        sf::Vector2f(button_width, button_height), "START GAME");
    start_btn->set_colors(sf::Color(30, 150, 50, 220), sf::Color(50, 200, 80, 255),
                          sf::Color(20, 120, 40, 255));
    start_btn->set_callback([this]() { on_start_game_clicked(); });
    m_buttons.push_back(std::move(start_btn));

    auto back_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing * 2),
        sf::Vector2f(button_width, button_height), "LEAVE LOBBY");
    back_btn->set_colors(sf::Color(120, 30, 40, 200), sf::Color(180, 50, 60, 255),
                         sf::Color(100, 20, 30, 255));
    back_btn->set_callback([this]() { on_back_clicked(); });
    m_buttons.push_back(std::move(back_btn));
}

void LobbyState::on_start_game_clicked() {
    std::cout << "[LobbyState] Start game button clicked\n";
    std::cout << "[LobbyState] Selected level: "
              << m_available_levels[static_cast<size_t>(m_selected_level_index)] << "\n";
    managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Plop);

    send_start_game_request();

    m_next_state = "game";
}

void LobbyState::on_level_select_clicked() {
    managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Plop);

    m_selected_level_index =
        (m_selected_level_index + 1) % static_cast<int>(m_available_levels.size());
    m_use_custom_level = (m_selected_level_index > 0);

    update_level_display();

    std::cout << "[LobbyState] Level selected: "
              << m_available_levels[static_cast<size_t>(m_selected_level_index)]
              << " (custom: " << (m_use_custom_level ? "yes" : "no") << ")\n";
}

void LobbyState::update_level_display() {
    auto window_size = m_window.getSize();
    sf::Vector2f center(static_cast<float>(window_size.x) / 2.0f,
                        static_cast<float>(window_size.y) / 2.0f);

    std::string level_name = m_available_levels[static_cast<size_t>(m_selected_level_index)];
    m_level_text.setString("Level: " + level_name);
    auto level_bounds = m_level_text.getLocalBounds();
    m_level_text.setPosition(center.x - level_bounds.width / 2.0f, center.y - 30.0f);
}

void LobbyState::on_back_clicked() {
    std::cout << "[LobbyState] Leave lobby button clicked\n";
    managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Plop);

    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LeaveLobby;
    serializer.compress();

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket);
    msg.raw_data = serializer.data();
    m_game_to_network_queue->push(msg);

    m_next_state = "lobby_list";
}

void LobbyState::send_start_game_request() {
    std::string level_id;
    if (m_selected_level_index >= 0 &&
        m_selected_level_index < static_cast<int>(m_available_level_ids.size())) {
        level_id = m_available_level_ids[static_cast<std::size_t>(m_selected_level_index)];
    }
    std::cout << "[LobbyState] Sending StartGame request with level_id: " << level_id << "\n";

    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::StartGame;
    serializer << static_cast<uint8_t>(level_id.size());
    for (char c : level_id) {
        serializer << static_cast<uint8_t>(c);
    }
    serializer.compress();

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket);
    msg.raw_data = serializer.data();
    m_game_to_network_queue->push(msg);
}

void LobbyState::send_keepalive() {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::Keepalive;
    serializer.compress();

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket);
    msg.raw_data = serializer.data();
    m_game_to_network_queue->push(msg);
}

void LobbyState::request_lobby_status() {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PlayerReady;
    serializer << static_cast<uint8_t>(0);
    serializer.compress();

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket);
    msg.raw_data = serializer.data();
    m_game_to_network_queue->push(msg);
}

void LobbyState::process_network_messages() {
    NetworkToGame::Message msg(NetworkToGame::MessageType::EntityUpdate);
    std::vector<NetworkToGame::Message> postponed_messages;

    while (m_network_to_game_queue->try_pop(msg)) {
        switch (msg.type) {
            case NetworkToGame::MessageType::LobbyStatus:
                std::cout << "[LobbyState] Received LobbyStatus: total=" << msg.total_players
                          << ", ready=" << msg.ready_players << std::endl;
                m_total_players = msg.total_players;
                m_ready_players = msg.ready_players;
                break;

            case NetworkToGame::MessageType::StartGame:
                std::cout << "[LobbyState] Game starting, transitioning to game state\n";
                m_next_state = "game";
                break;

            case NetworkToGame::MessageType::EntityUpdate:
                std::cout << "[LobbyState] Game started (received entity updates), transitioning\n";
                m_next_state = "game";
                break;

            case NetworkToGame::MessageType::ConnectionStatus:
                if (!msg.is_connected) {
                    std::cerr << "[LobbyState] Connection lost, returning to menu\n";
                    m_next_state = "menu";
                }
                break;

            case NetworkToGame::MessageType::LevelStart:
            case NetworkToGame::MessageType::LevelProgress:
            case NetworkToGame::MessageType::LevelComplete:
            case NetworkToGame::MessageType::PowerUpSelection:
            case NetworkToGame::MessageType::PowerUpCards:
            case NetworkToGame::MessageType::BossSpawn:
                postponed_messages.push_back(msg);
                break;

            default:
                break;
        }
    }

    for (const auto& postponed_msg : postponed_messages) {
        m_network_to_game_queue->push(postponed_msg);
    }
}

void LobbyState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        m_keyboard_navigation = false;
        sf::Vector2i pixel_pos(event.mouseMove.x, event.mouseMove.y);
        m_mouse_pos = m_window.mapPixelToCoords(pixel_pos);
        for (auto& button : m_buttons) {
            button->handle_mouse_move(m_mouse_pos);
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i pixel_pos(event.mouseButton.x, event.mouseButton.y);
            sf::Vector2f click_pos = m_window.mapPixelToCoords(pixel_pos);
            for (auto& button : m_buttons) {
                button->handle_mouse_click(click_pos);
            }
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            on_back_clicked();
        } else if (event.key.code == sf::Keyboard::Up) {
            m_keyboard_navigation = true;
            if (m_selected_button > 0) {
                m_selected_button--;
                managers::AudioManager::instance().play_sound(
                    managers::AudioManager::SoundType::Plop);
            }
        } else if (event.key.code == sf::Keyboard::Down) {
            m_keyboard_navigation = true;
            if (m_selected_button + 1 < m_buttons.size()) {
                m_selected_button++;
                managers::AudioManager::instance().play_sound(
                    managers::AudioManager::SoundType::Plop);
            }
        } else if (event.key.code == sf::Keyboard::Return ||
                   event.key.code == sf::Keyboard::Space) {
            if (m_selected_button < m_buttons.size()) {
                m_buttons[m_selected_button]->trigger();
            }
        }
    }
}

void LobbyState::update(float dt) {
    process_network_messages();

    m_keepalive_timer += dt;
    if (m_keepalive_timer >= KEEPALIVE_INTERVAL) {
        send_keepalive();
        m_keepalive_timer = 0.0f;
    }

    if (m_background) {
        m_background->update(dt);
    }
    if (m_title) {
        m_title->update(dt);
    }
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_keyboard_navigation) {
            // En mode clavier, reset tous puis highlight uniquement le sélectionné
            m_buttons[i]->set_hovered(i == m_selected_button);
        }
        m_buttons[i]->update(dt);
    }
    for (auto& corner : m_corners) {
        corner->update(dt);
    }
    for (auto& panel : m_side_panels) {
        panel->update(dt);
    }

    std::string status_info =
        "Joueurs: " + std::to_string(m_total_players) + "/" + std::to_string(m_max_players);
    m_status_text.setString(status_info);

    auto status_bounds = m_status_text.getLocalBounds();
    auto window_size = m_window.getSize();
    sf::Vector2f center(static_cast<float>(window_size.x) / 2.0f,
                        static_cast<float>(window_size.y) / 2.0f);
    m_status_text.setPosition(center.x - status_bounds.width / 2.0f, center.y - 100.0f);
}

void LobbyState::render(sf::RenderWindow& window) {
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

    window.draw(m_status_text);
    window.draw(m_level_text);

    for (auto& button : m_buttons) {
        button->render(window);
    }

    if (m_footer) {
        m_footer->render(window);
    }

    // Appliquer le shader colorblind
    rendering::ColorBlindShader::instance().apply(window);

    // Ancien système d'overlay désactivé - on utilise maintenant le shader GLSL
    /*ColorBlindMode mode = Settings::instance().colorblind_mode;
    if (mode != ColorBlindMode::Normal) {
        sf::RectangleShape overlay(sf::Vector2f(1920, 1080));

        switch (mode) {
            case ColorBlindMode::Protanopia:
                overlay.setFillColor(sf::Color(255, 255, 0, 90));
                break;
            case ColorBlindMode::Deuteranopia:
                overlay.setFillColor(sf::Color(255, 100, 255, 90));
                break;
            case ColorBlindMode::Tritanopia:
                overlay.setFillColor(sf::Color(255, 100, 50, 90));
                break;
            case ColorBlindMode::HighContrast:
                overlay.setFillColor(sf::Color(150, 150, 255, 110));
                break;
            default:
                break;
        }

        window.draw(overlay);
    }*/
}

}  // namespace rtype
