#pragma once

#include "game/Entity.hpp"
#include "managers/EffectsManager.hpp"
#include "managers/TextureManager.hpp"

#include <SFML/Graphics.hpp>

#include <map>

namespace rendering {

class GameRenderer {
public:
    static constexpr unsigned int WINDOW_WIDTH = 1920;
    static constexpr unsigned int WINDOW_HEIGHT = 1080;

    GameRenderer();
    ~GameRenderer() = default;

    void init(sf::RenderWindow& window);

    void update(float dt);

    void render_background(sf::RenderWindow& window);

    void set_background_level(uint8_t level);
    void render_level_transition(sf::RenderWindow& window);
    bool is_transitioning() const { return transition_active_; }

    void render_entities(sf::RenderWindow& window, std::map<uint32_t, Entity>& entities,
                         uint32_t my_network_id, float dt, float predicted_x = -1.0f, float predicted_y = -1.0f);

    void render_effects(sf::RenderWindow& window);

    void render_damage_flash(sf::RenderWindow& window);

    void render_colorblind_overlay(sf::RenderWindow& window);

    sf::View& get_game_view() { return game_view_; }
    void apply_screen_shake(sf::RenderWindow& window);
    void restore_view(sf::RenderWindow& window);

    GameRenderer(const GameRenderer&) = delete;
    GameRenderer& operator=(const GameRenderer&) = delete;

private:
    void update_ship_tilt(Entity& entity, float dt);
    void update_ally_tilt(Entity& entity, float dt);

    sf::Sprite bg_sprite1_;
    sf::Sprite bg_sprite2_;
    float bg_scroll_offset_ = 0.0f;
    const float bg_scroll_speed_ = 50.0f;
    std::vector<sf::Sprite> ruins_top_sprites_;
    std::vector<sf::Sprite> ruins_bottom_sprites_;
    std::vector<float> ruins_top_base_positions_;
    std::vector<float> ruins_bottom_base_positions_;
    float ruins_top_offset_ = 0.0f;
    float ruins_bottom_offset_ = 0.0f;
    float ruins_top_total_width_ = 0.0f;
    float ruins_bottom_total_width_ = 0.0f;
    sf::RectangleShape ruins_background_;
    bool transition_active_ = false;
    float transition_timer_ = 0.0f;
    float transition_duration_ = 3.0f;
    uint8_t current_bg_level_ = 1;
    uint8_t target_bg_level_ = 1;
    sf::RectangleShape transition_overlay_;
    sf::Text transition_text_;
    sf::Font transition_font_;

    sf::View game_view_;
};

}  // namespace rendering
