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

    sf::View game_view_;
};

}  // namespace rendering
