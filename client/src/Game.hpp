#pragma once

#include "Messages.hpp"
#include "NetworkClient.hpp"
#include "SafeQueue.hpp"

#include <SFML/Graphics.hpp>

#include <map>
#include <memory>
#include <string>

class Game {
private:
    static constexpr unsigned int WINDOW_WIDTH = 1920;
    static constexpr unsigned int WINDOW_HEIGHT = 1080;
    static constexpr unsigned int FRAMERATE = 60;

    ThreadSafeQueue<GameToNetwork::Message>& game_to_network_queue_;
    ThreadSafeQueue<NetworkToGame::Message>& network_to_game_queue_;

    sf::RenderWindow window_;
    sf::Font font_;
    sf::Text info_text_;

    bool is_running_;
    bool prev_space_pressed_ = false;
    std::map<uint32_t, Entity> entities_;

    void process_events();
    void handle_input();
    void update();
    void process_network_messages();
    void render();
    void setup_ui();

public:
    Game(ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
         ThreadSafeQueue<NetworkToGame::Message>& net_to_game);
    ~Game();

    void run();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
};
