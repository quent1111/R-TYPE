#pragma once

#include "common/SafeQueue.hpp"
#include "network/Messages.hpp"
#include "network/NetworkClient.hpp"
#include "states/IState.hpp"
#include "ui/MenuComponents.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <thread>
#include <vector>

namespace rtype {

class LobbyState : public IState {
public:
    LobbyState(sf::RenderWindow& window,
               std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> game_to_net,
               std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> net_to_game);
    ~LobbyState() override;

    void on_enter() override;
    void on_exit() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    std::string get_next_state() const override { return m_next_state; }
    void clear_next_state() override { m_next_state.clear(); }

private:
    void setup_ui();
    void on_ready_clicked();
    void on_back_clicked();
    void send_ready_status();
    void process_network_messages();

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

    sf::Text m_status_text;
    sf::Text m_player_count_text;
    sf::Text m_waiting_text;
    sf::Font m_font;

    bool m_is_ready{false};
    int m_total_players{0};
    int m_ready_players{0};

    sf::Vector2f m_mouse_pos;
};

}  // namespace rtype
