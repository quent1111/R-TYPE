#pragma once

#include "states/IState.hpp"
#include "Game.hpp"
#include "NetworkClient.hpp"
#include "SafeQueue.hpp"
#include "Messages.hpp"

#include <SFML/Graphics.hpp>
#include <memory>
#include <thread>

namespace rtype {

class GameState : public IState {
public:
    GameState(sf::RenderWindow& window,
              std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> game_to_net,
              std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> net_to_game);
    ~GameState() override;

    void on_enter() override;
    void on_exit() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;
    std::string get_next_state() const override { return m_next_state; }
    void clear_next_state() override { m_next_state.clear(); }

private:
    sf::RenderWindow& m_window;
    std::string m_next_state;
    
    std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> m_game_to_network_queue;
    std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> m_network_to_game_queue;
    
    std::unique_ptr<Game> m_game;
};

} // namespace rtype

