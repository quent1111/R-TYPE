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

    // Nettoyer tous les composants UI avant de les recréer
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

    // Background animé
    m_background = std::make_unique<ui::MenuBackground>(window_size);

    // Titre stylisé
    m_title =
        std::make_unique<ui::MenuTitle>("LOBBY", sf::Vector2f(center.x, center.y - 280.0f), 84);

    // Footer
    m_footer = std::make_unique<ui::MenuFooter>(window_size);

    // Décorations des coins
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

    // Panneaux latéraux
    m_side_panels.push_back(
        std::make_unique<ui::SidePanel>(sf::Vector2f(50.0f, center.y - 50.0f), true));
    m_side_panels.push_back(std::make_unique<ui::SidePanel>(
        sf::Vector2f(static_cast<float>(window_size.x) - 50.0f, center.y - 50.0f), false));

    // Textes d'info avec style amélioré
    m_status_text.setFont(m_font);
    m_status_text.setCharacterSize(28);
    m_status_text.setFillColor(sf::Color(100, 200, 255));
    m_status_text.setString(">> Connected to server <<");
    m_status_text.setOutlineColor(sf::Color(0, 50, 100));
    m_status_text.setOutlineThickness(1.5f);
    sf::FloatRect status_bounds = m_status_text.getLocalBounds();
    m_status_text.setOrigin(status_bounds.width / 2.0f, status_bounds.height / 2.0f);
    m_status_text.setPosition(center.x, center.y - 160.0f);

    m_player_count_text.setFont(m_font);
    m_player_count_text.setCharacterSize(48);
    m_player_count_text.setFillColor(sf::Color(255, 220, 100));
    m_player_count_text.setString("0 / 0 READY");
    m_player_count_text.setOutlineColor(sf::Color(100, 80, 0));
    m_player_count_text.setOutlineThickness(2.0f);
    sf::FloatRect count_bounds = m_player_count_text.getLocalBounds();
    m_player_count_text.setOrigin(count_bounds.width / 2.0f, count_bounds.height / 2.0f);
    m_player_count_text.setPosition(center.x, center.y - 80.0f);

    m_waiting_text.setFont(m_font);
    m_waiting_text.setCharacterSize(22);
    m_waiting_text.setFillColor(sf::Color(180, 180, 200));
    m_waiting_text.setString("--- Waiting for players ---");
    m_waiting_text.setStyle(sf::Text::Italic);
    sf::FloatRect waiting_bounds = m_waiting_text.getLocalBounds();
    m_waiting_text.setOrigin(waiting_bounds.width / 2.0f, waiting_bounds.height / 2.0f);
    m_waiting_text.setPosition(center.x, center.y + 200.0f);

    const float button_width = 380.0f;
    const float button_height = 75.0f;
    const float button_spacing = 95.0f;
    const float start_y = center.y + 30.0f;

    auto ready_btn =
        std::make_unique<ui::Button>(sf::Vector2f(center.x - button_width / 2.0f, start_y),
                                     sf::Vector2f(button_width, button_height), "READY");
    ready_btn->set_colors(sf::Color(30, 150, 50, 220), sf::Color(50, 200, 80, 255),
                          sf::Color(20, 120, 40, 255));
    ready_btn->set_callback([this]() { on_ready_clicked(); });
    m_buttons.push_back(std::move(ready_btn));

    auto back_btn = std::make_unique<ui::Button>(
        sf::Vector2f(center.x - button_width / 2.0f, start_y + button_spacing),
        sf::Vector2f(button_width, button_height), "BACK TO MENU");
    back_btn->set_colors(sf::Color(120, 30, 40, 200), sf::Color(180, 50, 60, 255),
                         sf::Color(100, 20, 30, 255));
    back_btn->set_callback([this]() { on_back_clicked(); });
    m_buttons.push_back(std::move(back_btn));
}

void LobbyState::on_ready_clicked() {
    std::cout << "[LobbyState] Ready button clicked\n";
    m_is_ready = !m_is_ready;

    if (m_is_ready) {
        m_buttons[0]->set_text("NOT READY");
        m_buttons[0]->set_colors(sf::Color(150, 100, 30, 220), sf::Color(200, 150, 50, 255),
                                 sf::Color(120, 80, 20, 255));
        m_status_text.setString("You are READY!");
        m_status_text.setFillColor(sf::Color(100, 255, 100));
    } else {
        m_buttons[0]->set_text("READY");
        m_buttons[0]->set_colors(sf::Color(30, 150, 50, 220), sf::Color(50, 200, 80, 255),
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
    m_game_to_network_queue->push(
        GameToNetwork::Message(GameToNetwork::MessageType::SendReady, m_is_ready));
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
            m_next_state = "menu";
        }
    }
}

void LobbyState::update(float dt) {
    process_network_messages();

    // Animation de tous les composants UI
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

    // Mise à jour des infos de lobby avec style
    std::string count_str =
        std::to_string(m_ready_players) + " / " + std::to_string(m_total_players) + " READY";
    m_player_count_text.setString(count_str);

    // Couleur qui change selon le statut
    if (m_ready_players == m_total_players && m_total_players > 0) {
        m_player_count_text.setFillColor(sf::Color(100, 255, 100));  // Vert quand prêt
    } else {
        m_player_count_text.setFillColor(sf::Color(255, 220, 100));  // Jaune sinon
    }

    sf::FloatRect bounds = m_player_count_text.getLocalBounds();
    m_player_count_text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
}

void LobbyState::render(sf::RenderWindow& window) {
    // Fond animé
    if (m_background) {
        m_background->render(window);
    }

    // Panneaux latéraux (derrière tout)
    for (auto& panel : m_side_panels) {
        panel->render(window);
    }

    // Décorations des coins
    for (auto& corner : m_corners) {
        corner->render(window);
    }

    // Titre stylisé
    if (m_title) {
        m_title->render(window);
    }

    // Compteur de joueurs uniquement
    window.draw(m_player_count_text);

    // Boutons
    for (auto& button : m_buttons) {
        button->render(window);
    }

    // Footer (au-dessus de tout)
    if (m_footer) {
        m_footer->render(window);
    }
}

}  // namespace rtype
