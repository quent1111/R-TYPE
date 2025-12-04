#include "states/GameState.hpp"
#include <iostream>

namespace rtype {

GameState::GameState(sf::RenderWindow& window,
                     std::shared_ptr<ThreadSafeQueue<GameToNetwork::Message>> game_to_net,
                     std::shared_ptr<ThreadSafeQueue<NetworkToGame::Message>> net_to_game) 
    : m_window(window),
      m_game_to_network_queue(game_to_net),
      m_network_to_game_queue(net_to_game) {}

GameState::~GameState() {
    on_exit();
}

void GameState::on_enter() {
    std::cout << "[GameState] Entering game\n";
    
    m_game = std::make_unique<Game>(
        m_window,
        *m_game_to_network_queue,
        *m_network_to_game_queue
    );
}

void GameState::on_exit() {
    std::cout << "[GameState] Exiting game\n";
    m_game.reset();
}

void GameState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            std::cout << "[GameState] Escape pressed - going back to menu\n";
            m_next_state = "menu";
        }
    }
    
    if (m_game) {
        if (event.type == sf::Event::GainedFocus) {
            m_game->set_focus(true);
        } else if (event.type == sf::Event::LostFocus) {
            m_game->set_focus(false);
        }
    }
}

void GameState::update(float /*dt*/) {
    if (m_game) {
        m_game->update();
        m_game->handle_input();
        
        if (!m_game->is_running()) {
            m_next_state = "menu";
        }
    }
}

void GameState::render(sf::RenderWindow& ) {
    if (m_game) {
        m_game->render();
    }
}

} // namespace rtype

