#pragma once

#include "AdminClient.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class AdminUI {
public:
    AdminUI(sf::RenderWindow& window, std::shared_ptr<AdminClient> client);

    void handle_event(const sf::Event& event);
    void update(float dt);
    void render();

    void refresh_data();

private:
    sf::RenderWindow& _window;
    std::shared_ptr<AdminClient> _client;
    sf::Font _font;

    sf::Text _title;
    sf::RectangleShape _status_panel;
    sf::Text _status_text;
    sf::RectangleShape _players_panel;
    sf::Text _players_title;
    std::vector<sf::Text> _player_texts;
    sf::RectangleShape _lobbies_panel;
    sf::Text _lobbies_title;
    std::vector<sf::Text> _lobby_texts;

    std::vector<PlayerInfo> _players;
    std::vector<LobbyInfo> _lobbies;
    ServerStatus _server_status;

    sf::Vector2f _mouse_pos;

    float _refresh_timer;
    const float REFRESH_INTERVAL = 2.0f;

    void setup_ui();
    void draw_status_panel();
    void draw_players_panel();
    void draw_lobbies_panel();
    void draw_action_buttons();

    void on_refresh_clicked();
    void on_close_lobby(int lobby_id);
};
