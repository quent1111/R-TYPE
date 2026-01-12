#pragma once

#include "common/SafeQueue.hpp"
#include "network/Messages.hpp"
#include "states/IState.hpp"
#include "ui/MenuComponents.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>
#include <vector>

namespace rtype {

struct LobbyInfo {
    int lobby_id;
    std::string name;
    int current_players;
    int max_players;
    std::string state_text;
};

class LobbyListState : public IState {
public:
    LobbyListState(sf::RenderWindow& window,
                   std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> game_to_net,
                   std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> net_to_game);
    ~LobbyListState() override;

    void on_enter() override;
    void on_exit() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    std::string get_next_state() const override { return m_next_state; }
    void clear_next_state() override { m_next_state.clear(); }

private:
    void setup_ui();
    void request_lobby_list();
    void on_refresh_clicked();
    void on_create_clicked();
    void on_play_clicked();
    void on_join_clicked(int lobby_id);
    void on_back_clicked();
    void process_network_messages();
    void update_lobby_list_ui();
    void send_create_lobby_request(const std::string& lobby_name);
    void send_join_lobby_request(int lobby_id);

    sf::RenderWindow& m_window;
    std::string m_next_state;

    std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> m_game_to_network_queue;
    std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> m_network_to_game_queue;

    std::unique_ptr<ui::MenuBackground> m_background;
    std::unique_ptr<ui::MenuTitle> m_title;
    std::unique_ptr<ui::MenuFooter> m_footer;
    std::vector<std::unique_ptr<ui::Button>> m_buttons;
    std::vector<std::unique_ptr<ui::CornerDecoration>> m_corners;
    std::vector<std::unique_ptr<ui::SidePanel>> m_side_panels;

    std::vector<LobbyInfo> m_lobbies;
    std::vector<std::unique_ptr<ui::Button>> m_lobby_buttons;

    sf::Text m_title_text;
    sf::Text m_info_text;
    sf::Font m_font;

    float m_refresh_timer = 0.0f;
    int m_selected_lobby_id = -1;

    sf::Vector2f m_mouse_pos;

    bool m_creating_lobby{false};
    bool m_matchmaking{false};
    std::string m_new_lobby_name;
    sf::Text m_input_text;
    sf::RectangleShape m_input_box;
    sf::Text m_input_label;
};

}  // namespace rtype
