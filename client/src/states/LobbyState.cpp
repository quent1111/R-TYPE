#include "states/LobbyState.hpp"

#include "managers/AudioManager.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"

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

    setup_ui();
}

void LobbyState::on_exit() {
    std::cout << "[LobbyState] Exiting lobby\n";
}

void LobbyState::setup_ui() {
    auto window_size = m_window.getSize();
    sf::Vector2f center(static_cast<float>(window_size.x) / 2.0f,
                        static_cast<float>(window_size.y) / 2.0f);

    m_background = std::make_unique<ui::MenuBackground>(window_size);

    m_title =
        std::make_unique<ui::MenuTitle>("LOBBY", sf::Vector2f(center.x, 50.0f), 72);

    m_footer = std::make_unique<ui::MenuFooter>(window_size);

    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(30.0f, 30.0f), false, false));
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
        sf::Vector2f(static_cast<float>(window_size.x) - 50.0f, static_cast<float>(window_size.y) / 2.0f - 50.0f), false));

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
    m_status_text.setPosition(center.x - status_bounds.width / 2.0f, center.y - 50.0f);

    const float button_width = 300.0f;
    const float button_height = 60.0f;
    const float button_spacing = 80.0f;
    const float start_y = center.y + 100.0f;

    auto start_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y),
        sf::Vector2f(button_width, button_height), "START GAME");
    start_btn->set_colors(sf::Color(30, 150, 50, 220), sf::Color(50, 200, 80, 255),
                          sf::Color(20, 120, 40, 255));
    start_btn->set_callback([this]() { on_start_game_clicked(); });
    m_buttons.push_back(std::move(start_btn));

    auto back_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing),
        sf::Vector2f(button_width, button_height), "LEAVE LOBBY");
    back_btn->set_colors(sf::Color(120, 30, 40, 200), sf::Color(180, 50, 60, 255),
                         sf::Color(100, 20, 30, 255));
    back_btn->set_callback([this]() { on_back_clicked(); });
    m_buttons.push_back(std::move(back_btn));
}

void LobbyState::on_start_game_clicked() {
    std::cout << "[LobbyState] Start game button clicked\n";
    managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Plop);

    send_start_game_request();

    m_next_state = "game";
}

void LobbyState::on_back_clicked() {
    std::cout << "[LobbyState] Leave lobby button clicked\n";
    managers::AudioManager::instance().play_sound(managers::AudioManager::SoundType::Plop);

    // Envoyer la requête de quitter le lobby au serveur
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LeaveLobby;

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket);
    msg.raw_data = serializer.data();
    m_game_to_network_queue->push(msg);

    m_next_state = "lobby_list";
}

void LobbyState::send_start_game_request() {
    std::cout << "[LobbyState] Sending StartGame request to server\n";

    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << static_cast<uint8_t>(RType::OpCode::StartGame);

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket);
    msg.raw_data = serializer.data();
    m_game_to_network_queue->push(msg);
}

void LobbyState::process_network_messages() {
    NetworkToGame::Message msg(NetworkToGame::MessageType::EntityUpdate);

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

            default:
                break;
        }
    }
}

void LobbyState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
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
        }
    }
}

void LobbyState::update(float dt) {
    process_network_messages();

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

    // Mise à jour simple du texte au centre
    std::string status_info = "Joueurs: " + std::to_string(m_total_players) + "/" + std::to_string(m_max_players);
    m_status_text.setString(status_info);
    
    auto status_bounds = m_status_text.getLocalBounds();
    auto window_size = m_window.getSize();
    sf::Vector2f center(static_cast<float>(window_size.x) / 2.0f,
                        static_cast<float>(window_size.y) / 2.0f);
    m_status_text.setPosition(center.x - status_bounds.width / 2.0f, center.y - 50.0f);
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

    for (auto& button : m_buttons) {
        button->render(window);
    }

    if (m_footer) {
        m_footer->render(window);
    }
}

}  // namespace rtype
