#pragma once

#include "network/Messages.hpp"
#include "network/NetworkClient.hpp"
#include "common/SafeQueue.hpp"
#include "input/InputHandler.hpp"
#include "managers/Managers.hpp"
#include "rendering/Rendering.hpp"

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

    sf::RenderWindow& window_;
    sf::Font font_;

    rendering::GameRenderer game_renderer_;
    rendering::HUDRenderer hud_renderer_;
    rendering::OverlayRenderer overlay_renderer_;
    input::InputHandler input_handler_;

    bool is_running_;
    bool has_focus_ = true;
    bool show_game_over_ = false;
    float game_over_timer_ = 0.0f;
    float game_over_duration_ = 3.0f;
    std::map<uint32_t, Entity> entities_;
    uint32_t my_network_id_ = 0;

    uint8_t current_level_ = 1;
    uint16_t enemies_killed_ = 0;
    uint16_t enemies_needed_ = 20;
    bool show_level_intro_ = false;
    float level_intro_timer_ = 0.0f;
    float level_intro_duration_ = 3.0f;

    bool show_powerup_selection_ = false;
    uint8_t powerup_type_ = 0;
    float powerup_time_remaining_ = 0.0f;
    bool has_x_key_been_released_ = true;
    std::map<uint32_t, std::pair<uint8_t, float>> player_powerups_;
    std::map<uint32_t, int> player_shield_frame_;
    std::map<uint32_t, float> player_shield_anim_timer_;

    sf::Sprite powerup_card1_sprite_;
    sf::Sprite powerup_card2_sprite_;

    uint32_t current_score_ = 0;
    uint32_t displayed_score_ = 0;

    float game_time_ = 0.0f;
    bool timer_running_ = false;

    uint16_t prev_enemies_killed_ = 0;
    int prev_player_health_ = -1;

    void process_network_messages();
    void setup_ui();
    void setup_input_handler();

    void init_entity_sprite(Entity& entity);

public:
    Game(sf::RenderWindow& window, ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
         ThreadSafeQueue<NetworkToGame::Message>& net_to_game);
    ~Game();

    void run();
    void handle_input();
    void handle_event(const sf::Event& event);
    void update();
    void render();
    bool is_running() const { return is_running_; }
    void set_focus(bool focus) { has_focus_ = focus; }

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
};
