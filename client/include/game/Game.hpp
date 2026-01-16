#pragma once

#include "../../game-lib/include/powerup/PowerupRegistry.hpp"
#include "common/SafeQueue.hpp"
#include "input/InputHandler.hpp"
#include "level/CustomLevelConfig.hpp"
#include "managers/Managers.hpp"
#include "network/Messages.hpp"
#include "network/NetworkClient.hpp"
#include "rendering/Rendering.hpp"
#include "ui/SettingsPanel.hpp"

#include <SFML/Graphics.hpp>

#include <map>
#include <memory>
#include <optional>
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
    bool powerup_choice_pending_ = false;
    std::map<std::pair<uint32_t, uint8_t>, float> player_powerups_;
    std::map<uint32_t, int> player_shield_frame_;
    std::map<uint32_t, float> player_shield_anim_timer_;
    std::vector<NetworkToGame::Message::PowerUpCard> powerup_cards_;

    std::vector<std::pair<std::optional<powerup::PowerupId>, uint8_t>> my_activable_slots_;
    std::vector<float> my_slot_timers_;
    std::vector<float> my_slot_cooldowns_;
    std::vector<bool> my_slot_active_;

    sf::Sprite powerup_card1_sprite_;
    sf::Sprite powerup_card2_sprite_;
    sf::Sprite powerup_card3_sprite_;

    uint32_t current_score_ = 0;
    uint32_t displayed_score_ = 0;

    float game_time_ = 0.0f;
    bool timer_running_ = false;

    uint16_t prev_enemies_killed_ = 0;
    int prev_player_health_ = -1;

    bool boss_spawn_triggered_ = false;
    float boss_roar_timer_ = 0.0f;
    float boss_roar_delay_ = 2.5f;
    std::unordered_map<uint32_t, float> prev_boss_damage_timer_;
    
    int boss_explosion_count_ = 0;

    std::optional<level::CustomLevelConfig> custom_level_config_;
    std::string current_custom_level_id_;

    float predicted_player_x_ = 0.0f;
    float predicted_player_y_ = 0.0f;
    uint8_t last_input_mask_ = 0;
    bool has_server_position_ = false;

    void process_network_messages();
    void setup_ui();
    void setup_input_handler();
    void request_game_state();

    void init_entity_sprite(Entity& entity, uint32_t entity_id);
    void load_custom_level(const std::string& level_id);
    void load_custom_level_textures();

    std::unique_ptr<rtype::ui::SettingsPanel> m_settings_panel;
    bool m_request_return_to_menu{false};

public:
    void request_return_to_menu() { m_request_return_to_menu = true; }
    bool should_return_to_menu() const { return m_request_return_to_menu; }

public:
    Game(sf::RenderWindow& window, ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
         ThreadSafeQueue<NetworkToGame::Message>& net_to_game);
    ~Game();

    void run();
    void handle_input(float dt = 0.0f);
    void handle_event(const sf::Event& event);
    void update();
    void render();
    bool is_running() const { return is_running_; }
    void set_focus(bool focus) { has_focus_ = focus; }

private:
    void update_powerup_card_sprites();

public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
};
