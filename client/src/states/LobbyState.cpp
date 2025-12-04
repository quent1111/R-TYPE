#include "states/LobbyState.hpp"
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

    m_title = std::make_unique<ui::MenuTitle>(
        "LOBBY", sf::Vector2f(center.x, center.y - 220.0f), 72);

    m_status_text.setFont(m_font);
    m_status_text.setCharacterSize(30);
    m_status_text.setFillColor(sf::Color::White);
    m_status_text.setString("Connecting to server...");
    sf::FloatRect status_bounds = m_status_text.getLocalBounds();
    m_status_text.setOrigin(status_bounds.width / 2.0f, status_bounds.height / 2.0f);
    m_status_text.setPosition(center.x, center.y - 100.0f);

    m_player_count_text.setFont(m_font);
    m_player_count_text.setCharacterSize(24);
    m_player_count_text.setFillColor(sf::Color(150, 150, 255));
    m_player_count_text.setString("Players: 0 | Ready: 0");
    sf::FloatRect count_bounds = m_player_count_text.getLocalBounds();
    m_player_count_text.setOrigin(count_bounds.width / 2.0f, count_bounds.height / 2.0f);
    m_player_count_text.setPosition(center.x, center.y - 50.0f);

    m_waiting_text.setFont(m_font);
    m_waiting_text.setCharacterSize(20);
    m_waiting_text.setFillColor(sf::Color(200, 200, 200));
    m_waiting_text.setString("Waiting for all players to be ready...");
    sf::FloatRect waiting_bounds = m_waiting_text.getLocalBounds();
    m_waiting_text.setOrigin(waiting_bounds.width / 2.0f, waiting_bounds.height / 2.0f);
    m_waiting_text.setPosition(center.x, center.y + 150.0f);

    const float button_width = 340.0f;
    const float button_height = 70.0f;
    const float button_spacing = 90.0f;
    const float start_y = center.y + 20.0f;

    auto ready_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y),
        sf::Vector2f(button_width, button_height), "READY");
    ready_btn->set_colors(sf::Color(30, 150, 50, 220),
                          sf::Color(50, 200, 80, 255),
                          sf::Color(20, 120, 40, 255));
    ready_btn->set_callback([this]() { on_ready_clicked(); });
    m_buttons.push_back(std::move(ready_btn));

    auto back_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing),
        sf::Vector2f(button_width, button_height), "BACK");
    back_btn->set_colors(sf::Color(120, 30, 40, 200),
                         sf::Color(180, 50, 60, 255),
                         sf::Color(100, 20, 30, 255));
    back_btn->set_callback([this]() { on_back_clicked(); });
    m_buttons.push_back(std::move(back_btn));
}

void LobbyState::on_ready_clicked() {
    std::cout << "[LobbyState] Ready button clicked\n";
    m_is_ready = !m_is_ready;
    
    if (m_is_ready) {
        m_buttons[0]->set_text("NOT READY");
        m_buttons[0]->set_colors(sf::Color(150, 100, 30, 220),
                                 sf::Color(200, 150, 50, 255),
                                 sf::Color(120, 80, 20, 255));
        m_status_text.setString("You are READY!");
        m_status_text.setFillColor(sf::Color(100, 255, 100));
    } else {
        m_buttons[0]->set_text("READY");
        m_buttons[0]->set_colors(sf::Color(30, 150, 50, 220),
                                 sf::Color(50, 200, 80, 255),
                                 sf::Color(20, 120, 40, 255));
        m_status_text.setString("Click READY to start");
        m_status_text.setFillColor(sf::Color::White);
    }
    
    send_ready_status();
}

void LobbyState::on_back_clicked() {
    std::cout << "[LobbyState] Back button clicked\n";
    m_next_state = "menu";
}

void LobbyState::send_ready_status() {
    m_game_to_network_queue->push(GameToNetwork::Message(GameToNetwork::MessageType::SendReady, m_is_ready));
}

void LobbyState::process_network_messages() {
    NetworkToGame::Message msg(NetworkToGame::MessageType::EntityUpdate);
    
    while (m_network_to_game_queue->try_pop(msg)) {
        switch (msg.type) {
            case NetworkToGame::MessageType::LobbyStatus:
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
        m_mouse_pos = sf::Vector2f(static_cast<float>(event.mouseMove.x),
                                    static_cast<float>(event.mouseMove.y));
        for (auto& button : m_buttons) {
            button->handle_mouse_move(m_mouse_pos);
        }
    }

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f click_pos(static_cast<float>(event.mouseButton.x),
                                   static_cast<float>(event.mouseButton.y));
            for (auto& button : m_buttons) {
                button->handle_mouse_click(click_pos);
            }
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            m_next_state = "menu";
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
    
    std::string count_str = "Players: " + std::to_string(m_total_players) + 
                           " | Ready: " + std::to_string(m_ready_players);
    m_player_count_text.setString(count_str);
    sf::FloatRect bounds = m_player_count_text.getLocalBounds();
    m_player_count_text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
}

void LobbyState::render(sf::RenderWindow& window) {
    if (m_background) {
        m_background->render(window);
    }

    if (m_title) {
        m_title->render(window);
    }

    window.draw(m_status_text);
    window.draw(m_player_count_text);
    
    if (m_total_players > 0 && m_ready_players < m_total_players) {
        window.draw(m_waiting_text);
    }

    for (auto& button : m_buttons) {
        button->render(window);
    }
}

} // namespace rtype

