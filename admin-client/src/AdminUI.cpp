#include "AdminUI.hpp"
#include <iostream>
#include <sstream>

AdminUI::AdminUI(sf::RenderWindow& window, std::shared_ptr<AdminClient> client)
    : _window(window), _client(client), _refresh_timer(0.f) {

    if (!_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[AdminUI] Failed to load font" << std::endl;
    }

    setup_ui();
    refresh_data();
}

void AdminUI::setup_ui() {
    _title.setFont(_font);
    _title.setString("R-TYPE SERVER ADMINISTRATION");
    _title.setCharacterSize(36);
    _title.setFillColor(sf::Color::White);
    _title.setPosition(400, 20);

    _status_panel.setSize(sf::Vector2f(350, 150));
    _status_panel.setPosition(50, 100);
    _status_panel.setFillColor(sf::Color(30, 30, 30, 220));
    _status_panel.setOutlineColor(sf::Color(100, 100, 100));
    _status_panel.setOutlineThickness(2);

    _status_text.setFont(_font);
    _status_text.setCharacterSize(18);
    _status_text.setFillColor(sf::Color::White);
    _status_text.setPosition(60, 110);

    _players_panel.setSize(sf::Vector2f(700, 300));
    _players_panel.setPosition(50, 280);
    _players_panel.setFillColor(sf::Color(30, 30, 30, 220));
    _players_panel.setOutlineColor(sf::Color(100, 100, 100));
    _players_panel.setOutlineThickness(2);

    _players_title.setFont(_font);
    _players_title.setString("CONNECTED PLAYERS");
    _players_title.setCharacterSize(24);
    _players_title.setFillColor(sf::Color(100, 200, 255));
    _players_title.setPosition(60, 290);

    _lobbies_panel.setSize(sf::Vector2f(700, 200));
    _lobbies_panel.setPosition(50, 600);
    _lobbies_panel.setFillColor(sf::Color(30, 30, 30, 220));
    _lobbies_panel.setOutlineColor(sf::Color(100, 100, 100));
    _lobbies_panel.setOutlineThickness(2);

    _lobbies_title.setFont(_font);
    _lobbies_title.setString("ACTIVE LOBBIES");
    _lobbies_title.setCharacterSize(24);
    _lobbies_title.setFillColor(sf::Color(100, 255, 100));
    _lobbies_title.setPosition(60, 610);
}

void AdminUI::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        _mouse_pos = sf::Vector2f(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
    }
}

void AdminUI::update(float dt) {
    _refresh_timer += dt;

    if (_refresh_timer >= REFRESH_INTERVAL) {
        refresh_data();
        _refresh_timer = 0.f;
    }
}

void AdminUI::render() {
    draw_status_panel();
    draw_players_panel();
    draw_lobbies_panel();
    draw_action_buttons();
}

void AdminUI::draw_status_panel() {
    _window.draw(_status_panel);

    std::stringstream ss;
    ss << "SERVER STATUS\n\n"
       << "Uptime: " << _server_status.uptime << "\n"
       << "Players: " << _server_status.player_count << "\n"
       << "Lobbies: " << _server_status.lobby_count;

    _status_text.setString(ss.str());
    _window.draw(_status_text);
}

void AdminUI::draw_players_panel() {
    _window.draw(_players_panel);
    _window.draw(_players_title);

    _player_texts.clear();

    float y_offset = 330;

    for (const auto& player : _players) {
        sf::Text player_text;
        player_text.setFont(_font);
        player_text.setCharacterSize(16);
        player_text.setFillColor(sf::Color::White);

        std::stringstream ss;
        ss << "ID: " << player.id << "  |  "
           << player.address << ":" << player.port;
        player_text.setString(ss.str());
        player_text.setPosition(60, y_offset);

        _player_texts.push_back(player_text);
        _window.draw(player_text);

        y_offset += 25;
    }

    if (_players.empty()) {
        sf::Text no_players;
        no_players.setFont(_font);
        no_players.setString("No players connected");
        no_players.setCharacterSize(16);
        no_players.setFillColor(sf::Color(150, 150, 150));
        no_players.setPosition(60, 330);
        _window.draw(no_players);
    }
}

void AdminUI::draw_lobbies_panel() {
    _window.draw(_lobbies_panel);
    _window.draw(_lobbies_title);

    _lobby_texts.clear();

    float y_offset = 650;
    for (const auto& lobby : _lobbies) {
        sf::Text lobby_text;
        lobby_text.setFont(_font);
        lobby_text.setCharacterSize(16);
        lobby_text.setFillColor(sf::Color::White);

        std::stringstream ss;
        ss << "#" << lobby.id << " - " << lobby.name
           << "  (" << lobby.current_players << "/" << lobby.max_players << ")  "
           << (lobby.state == 2 ? "In Game" : "Waiting");
        lobby_text.setString(ss.str());
        lobby_text.setPosition(60, y_offset);

        _lobby_texts.push_back(lobby_text);
        _window.draw(lobby_text);

        y_offset += 25;
    }

    if (_lobbies.empty()) {
        sf::Text no_lobbies;
        no_lobbies.setFont(_font);
        no_lobbies.setString("No active lobbies");
        no_lobbies.setCharacterSize(16);
        no_lobbies.setFillColor(sf::Color(150, 150, 150));
        no_lobbies.setPosition(60, 650);
        _window.draw(no_lobbies);
    }
}

void AdminUI::draw_action_buttons() {
    _window.draw(_title);
}

void AdminUI::refresh_data() {
    if (!_client || !_client->is_authenticated()) {
        return;
    }

    _players = _client->get_players();
    _lobbies = _client->get_lobbies();
    _server_status = _client->get_server_status();

    std::cout << "[AdminUI] Data refreshed" << std::endl;
}

void AdminUI::on_refresh_clicked() {
    std::cout << "[AdminUI] Manual refresh requested" << std::endl;
    refresh_data();
    _refresh_timer = 0.f;
}

void AdminUI::on_close_lobby(int lobby_id) {
    std::string command = "close-lobby " + std::to_string(lobby_id);
    _client->send_command(command);
    refresh_data();
}
