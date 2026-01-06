#include "states/LobbyListState.hpp"

#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"

#include <iostream>

namespace rtype {

LobbyListState::LobbyListState(
    sf::RenderWindow& window,
    std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> game_to_net,
    std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> net_to_game)
    : m_window(window),
      m_game_to_network_queue(game_to_net),
      m_network_to_game_queue(net_to_game) {
    if (!m_font.loadFromFile("assets/fonts/PressStart2P-Regular.ttf")) {
        std::cerr << "[LobbyListState] Failed to load font" << std::endl;
    }
}

LobbyListState::~LobbyListState() = default;

void LobbyListState::on_enter() {
    std::cout << "[LobbyListState] Entering lobby list state" << std::endl;
    setup_ui();
    request_lobby_list();
}

void LobbyListState::on_exit() {
    std::cout << "[LobbyListState] Exiting lobby list state" << std::endl;
}

void LobbyListState::setup_ui() {
    auto window_size = m_window.getSize();

    m_background = std::make_unique<ui::MenuBackground>(window_size);

    m_title = std::make_unique<ui::MenuTitle>("LOBBIES",
                                               sf::Vector2f(window_size.x / 2.0f, 50.0f));
    m_footer = std::make_unique<ui::MenuFooter>(window_size);
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(30.0f, 30.0f), false, false));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(window_size.x - 30.0f, 30.0f), true, false));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(30.0f, window_size.y - 80.0f), false, true));
    m_corners.push_back(std::make_unique<ui::CornerDecoration>(
        sf::Vector2f(window_size.x - 30.0f, window_size.y - 80.0f), true, true));

    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(50.0f, window_size.y / 2.0f - 50.0f), true));
    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(window_size.x - 50.0f, window_size.y / 2.0f - 50.0f), false));

    float button_width = 250.0f;
    float button_height = 50.0f;
    float button_x = window_size.x / 2.0f - button_width / 2.0f;
    float bottom_y = window_size.y - 150.0f;

    auto create_btn = std::make_unique<ui::Button>(
        sf::Vector2f(button_x - 280.0f, bottom_y),
        sf::Vector2f(button_width, button_height),
        "CREATE LOBBY");
    create_btn->set_callback([this]() { on_create_clicked(); });
    m_buttons.push_back(std::move(create_btn));

    auto refresh_btn = std::make_unique<ui::Button>(
        sf::Vector2f(button_x, bottom_y),
        sf::Vector2f(button_width, button_height),
        "REFRESH");
    refresh_btn->set_callback([this]() { on_refresh_clicked(); });
    m_buttons.push_back(std::move(refresh_btn));

    auto back_btn = std::make_unique<ui::Button>(
        sf::Vector2f(button_x + 280.0f, bottom_y),
        sf::Vector2f(button_width, button_height),
        "BACK");
    back_btn->set_callback([this]() { on_back_clicked(); });
    m_buttons.push_back(std::move(back_btn));

    m_info_text.setFont(m_font);
    m_info_text.setCharacterSize(16);
    m_info_text.setFillColor(sf::Color(150, 150, 150));
    m_info_text.setString("Loading lobbies...");
    auto text_bounds = m_info_text.getLocalBounds();
    m_info_text.setPosition(
        window_size.x / 2.0f - text_bounds.width / 2.0f,
        200.0f
    );
}

void LobbyListState::request_lobby_list() {
    std::cout << "[LobbyListState] Requesting lobby list from server" << std::endl;
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::ListLobbies;

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket, serializer.data());
    m_game_to_network_queue->push(msg);
}

void LobbyListState::send_create_lobby_request(const std::string& lobby_name) {
    std::cout << "[LobbyListState] Creating lobby: " << lobby_name << std::endl;
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::CreateLobby;
    serializer << lobby_name;

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket, serializer.data());
    m_game_to_network_queue->push(msg);

    m_refresh_timer = 0.5f;
}

void LobbyListState::send_join_lobby_request(int lobby_id) {
    std::cout << "[LobbyListState] Joining lobby ID: " << lobby_id << std::endl;
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::JoinLobby;
    serializer << static_cast<int32_t>(lobby_id);

    GameToNetwork::Message msg(GameToNetwork::MessageType::RawPacket, serializer.data());
    m_game_to_network_queue->push(msg);

    m_next_state = "lobby";
}

void LobbyListState::on_refresh_clicked() {
    std::cout << "[LobbyListState] Refresh clicked" << std::endl;
    m_info_text.setString("Refreshing...");
    request_lobby_list();
}

void LobbyListState::on_create_clicked() {
    std::cout << "[LobbyListState] Create clicked" << std::endl;
    send_create_lobby_request("New Lobby");
    m_refresh_timer = 0.5f;
}

void LobbyListState::on_join_clicked(int lobby_id) {
    std::cout << "[LobbyListState] Join clicked for lobby " << lobby_id << std::endl;
    send_join_lobby_request(lobby_id);
}

void LobbyListState::on_back_clicked() {
    std::cout << "[LobbyListState] Back clicked" << std::endl;
    m_next_state = "menu";
}

void LobbyListState::update_lobby_list_ui() {
    auto window_size = m_window.getSize();
    m_lobby_buttons.clear();

    if (m_lobbies.empty()) {
        m_info_text.setString("No lobbies available. Create one!");
        auto text_bounds = m_info_text.getLocalBounds();
        m_info_text.setPosition(
            window_size.x / 2.0f - text_bounds.width / 2.0f,
            window_size.y / 2.0f - 100.0f
        );
        return;
    }

    m_info_text.setString("");

    float start_y = 150.0f;
    float button_width = 700.0f;
    float button_height = 60.0f;
    float spacing = 80.0f;
    float button_x = window_size.x / 2.0f - button_width / 2.0f;

    for (size_t i = 0; i < m_lobbies.size() && i < 6; ++i) {
        const auto& lobby = m_lobbies[i];
        float y = start_y + i * spacing;

        std::string button_text = lobby.name + "  [" +
                                  std::to_string(lobby.current_players) + "/" +
                                  std::to_string(lobby.max_players) + "]  " +
                                  lobby.state_text;

        auto lobby_btn = std::make_unique<ui::Button>(
            sf::Vector2f(button_x, y),
            sf::Vector2f(button_width, button_height),
            button_text);

        int lobby_id = lobby.lobby_id;
        lobby_btn->set_callback([this, lobby_id]() { on_join_clicked(lobby_id); });
        m_lobby_buttons.push_back(std::move(lobby_btn));
    }
}

void LobbyListState::process_network_messages() {
    NetworkToGame::Message msg(NetworkToGame::MessageType::LobbyListUpdate);
    while (m_network_to_game_queue->try_pop(msg)) {
        std::cout << "[LobbyListState] Received message type: " << static_cast<int>(msg.type) << std::endl;

        if (msg.type == NetworkToGame::MessageType::LobbyListUpdate && !msg.raw_lobby_data.empty()) {
            std::cout << "[LobbyListState] Processing LobbyListUpdate with " << msg.raw_lobby_data.size() << " bytes" << std::endl;

            RType::BinarySerializer deserializer(msg.raw_lobby_data);

            uint16_t magic = 0;
            deserializer >> magic;
            if (magic != RType::MagicNumber::VALUE) {
                std::cerr << "[LobbyListState] Invalid magic number: 0x" << std::hex << magic << std::dec << std::endl;
                std::cerr << "[LobbyListState] First 16 bytes: ";
                for (size_t i = 0; i < std::min(size_t(16), msg.raw_lobby_data.size()); ++i) {
                    std::cerr << std::hex << static_cast<int>(msg.raw_lobby_data[i]) << " ";
                }
                std::cerr << std::dec << std::endl;
                continue;
            }

            uint8_t opcode_value = 0;
            deserializer >> opcode_value;
            auto opcode = static_cast<RType::OpCode>(opcode_value);

            if (opcode == RType::OpCode::ListLobbies) {
                int32_t count = 0;
                deserializer >> count;

                std::cout << "[LobbyListState] Received " << count << " lobbies" << std::endl;

                if (count < 0 || count > 100) {
                    std::cerr << "[LobbyListState] Invalid lobby count: " << count << std::endl;
                    std::cerr << "[LobbyListState] Packet dump: ";
                    for (size_t i = 0; i < msg.raw_lobby_data.size(); ++i) {
                        std::cerr << std::hex << static_cast<int>(msg.raw_lobby_data[i]) << " ";
                    }
                    std::cerr << std::dec << std::endl;
                    continue;
                }

                m_lobbies.clear();
                for (int32_t i = 0; i < count; ++i) {
                    LobbyInfo info;
                    deserializer >> info.lobby_id;
                    deserializer >> info.name;
                    deserializer >> info.current_players;
                    deserializer >> info.max_players;
                    uint8_t state = 0;
                    deserializer >> state;
                    switch (state) {
                        case 0: info.state_text = "Waiting"; break;
                        case 1: info.state_text = "Ready"; break;
                        case 2: info.state_text = "In Game"; break;
                        case 3: info.state_text = "Finished"; break;
                        default: info.state_text = "Unknown"; break;
                    }
                    m_lobbies.push_back(info);
                }
                update_lobby_list_ui();
            }
        }
    }
}

void LobbyListState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        on_back_clicked();
        return;
    }

    if (event.type == sf::Event::MouseMoved) {
        m_mouse_pos = sf::Vector2f(static_cast<float>(event.mouseMove.x),
                                    static_cast<float>(event.mouseMove.y));
        for (auto& btn : m_buttons) {
            btn->handle_mouse_move(m_mouse_pos);
        }
        for (auto& btn : m_lobby_buttons) {
            btn->handle_mouse_move(m_mouse_pos);
        }
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f click_pos(static_cast<float>(event.mouseButton.x),
                               static_cast<float>(event.mouseButton.y));
        for (auto& btn : m_buttons) {
            btn->handle_mouse_click(click_pos);
        }
        for (auto& btn : m_lobby_buttons) {
            btn->handle_mouse_click(click_pos);
        }
    }
}

void LobbyListState::update(float dt) {
    if (m_background) m_background->update(dt);
    if (m_title) m_title->update(dt);

    for (auto& corner : m_corners) {
        corner->update(dt);
    }

    for (auto& panel : m_side_panels) {
        panel->update(dt);
    }

    for (auto& btn : m_buttons) {
        btn->update(dt);
    }

    for (auto& btn : m_lobby_buttons) {
        btn->update(dt);
    }

    if (m_refresh_timer > 0.0f) {
        m_refresh_timer -= dt;
        if (m_refresh_timer <= 0.0f) {
            request_lobby_list();
        }
    }
    process_network_messages();
}

void LobbyListState::render(sf::RenderWindow& window) {
    if (m_background) m_background->render(window);

    for (auto& corner : m_corners) {
        corner->render(window);
    }

    for (auto& panel : m_side_panels) {
        panel->render(window);
    }

    if (m_title) m_title->render(window);

    for (auto& btn : m_lobby_buttons) {
        btn->render(window);
    }

    for (auto& btn : m_buttons) {
        btn->render(window);
    }
    window.draw(m_info_text);
    if (m_footer) m_footer->render(window);
}

}  // namespace rtype
