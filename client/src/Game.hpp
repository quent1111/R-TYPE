#pragma once

#include "AudioManager.hpp"
#include "EffectsManager.hpp"
#include "Messages.hpp"
#include "NetworkClient.hpp"
#include "SafeQueue.hpp"
#include "TextureManager.hpp"

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
    sf::Text info_text_;
    TextureManager texture_manager_;

    sf::Sprite bg_sprite1_;
    sf::Sprite bg_sprite2_;
    float bg_scroll_offset_ = 0.0f;
    const float bg_scroll_speed_ = 50.0f;

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

    sf::Text level_intro_title_;
    sf::Text level_intro_subtitle_;
    sf::RectangleShape level_intro_overlay_;
    sf::Text level_text_;
    sf::Text progress_text_;
    sf::RectangleShape progress_bar_bg_;
    sf::RectangleShape progress_bar_fill_;

    sf::RectangleShape powerup_overlay_;
    sf::Text powerup_title_;
    sf::Sprite powerup_card1_sprite_;
    sf::Sprite powerup_card2_sprite_;
    sf::Text powerup_number1_text_;
    sf::Text powerup_number2_text_;
    sf::Text powerup_instruction_;
    sf::Text powerup_active_text_;
    sf::Sprite shield_visual_;
    std::vector<sf::IntRect> shield_frames_;
    std::map<uint32_t, int> player_shield_frame_;
    std::map<uint32_t, float> player_shield_anim_timer_;
    sf::Text powerup_hint_text_;
    sf::RectangleShape powerup_hint_bg_;

    sf::RectangleShape health_bar_bg_;
    sf::RectangleShape health_bar_fill_;
    sf::Text health_text_;

    sf::Sprite game_over_sprite_;
    sf::RectangleShape game_over_overlay_;

    sf::Text score_text_;
    uint32_t current_score_ = 0;
    uint32_t displayed_score_ = 0;

    sf::Text timer_text_;
    float game_time_ = 0.0f;
    bool timer_running_ = false;

    sf::Text combo_text_;
    sf::RectangleShape combo_bar_bg_;
    sf::RectangleShape combo_bar_fill_;
    sf::RectangleShape combo_timer_bar_;

    bool was_shooting_ = false;
    uint16_t prev_enemies_killed_ = 0;
    int prev_player_health_ = -1;

    void render_level_intro();
    void render_level_hud();
    void render_combo_bar();
    void render_powerup_selection();
    void render_powerup_active();
    void render_game_over();

    void process_network_messages();
    void setup_ui();

    void init_entity_sprite(Entity& entity);
    void update_ship_tilt(Entity& entity, float dt);

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
