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

    m_game = std::make_unique<Game>(m_window, *m_game_to_network_queue, *m_network_to_game_queue);
}

void GameState::on_exit() {
    std::cout << "[GameState] Exiting game\n";
    // Ensure the window view is reset when leaving the game so subsequent
    // menu rendering uses the default view (prevents cropped / shifted UI).
    try {
        m_window.setView(m_window.getDefaultView());
    } catch (...) {
        // defensive: ignore if window state is invalid
    }
    m_game.reset();
}

void GameState::handle_event(const sf::Event& event) {
    // Escape handled by Game (open settings). Do not force state change here.

    if (m_game) {
        if (event.type == sf::Event::GainedFocus) {
            m_game->set_focus(true);
        } else if (event.type == sf::Event::LostFocus) {
            m_game->set_focus(false);
        }
        m_game->handle_event(event);
    }
}

void GameState::update(float dt) {
    if (m_game) {
        m_game->update();
        m_game->handle_input(dt);

        if (!m_game->is_running()) {
            m_next_state = "menu";
        }

        // If game requested to return to menu (via settings Quit), transition
        if (m_game->should_return_to_menu()) {
            m_next_state = "menu";
        }
    }
}

void GameState::render(sf::RenderWindow&) {
    if (m_game) {
        m_game->render();
    }
}

}  // namespace rtype
