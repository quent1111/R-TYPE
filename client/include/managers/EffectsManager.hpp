#pragma once

#include <SFML/Graphics.hpp>

#include <deque>
#include <random>
#include <vector>

namespace managers {

class EffectsManager {
public:
    static EffectsManager& instance();

    EffectsManager(const EffectsManager&) = delete;
    EffectsManager& operator=(const EffectsManager&) = delete;

    void update(float dt);
    void render(sf::RenderWindow& window);

    void trigger_screen_shake(float intensity = 5.0f, float duration = 0.12f);
    sf::Vector2f get_screen_shake_offset() const;

    void spawn_score_particles(sf::Vector2f from_pos, sf::Vector2f to_pos, int count = 8);
    void spawn_explosion(sf::Vector2f position, int count = 20);

    void trigger_damage_flash();
    float get_damage_flash_alpha() const;

    void add_combo_kill();
    void reset_combo();
    int get_combo_multiplier() const;
    float get_combo_progress() const;
    float get_combo_timer() const;

    void trigger_score_bounce();
    float get_score_scale() const;

    void set_score_position(sf::Vector2f pos);

private:
    EffectsManager();
    ~EffectsManager() = default;

    float shake_intensity_ = 0.0f;
    float shake_duration_ = 0.0f;
    float shake_timer_ = 0.0f;
    sf::Vector2f shake_offset_;

    float score_scale_ = 1.0f;
    float score_bounce_timer_ = 0.0f;
    bool score_bouncing_ = false;

    struct ScoreParticle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Vector2f target;
        float lifetime;
        float max_lifetime;
        sf::Color color;
        float size;
        std::deque<sf::Vector2f> trail;
    };
    std::vector<ScoreParticle> score_particles_;
    sf::Vector2f score_position_;

    struct ExplosionParticle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float lifetime;
        float max_lifetime;
        sf::Color color;
        float size;
    };
    std::vector<ExplosionParticle> explosion_particles_;

    float damage_flash_timer_ = 0.0f;
    float damage_flash_duration_ = 0.2f;

    int combo_kills_ = 0;
    float combo_timer_ = 0.0f;
    float combo_decay_time_ = 5.0f;
    int combo_multiplier_ = 1;
    int kills_per_level_ = 3;

    sf::CircleShape particle_shape_;
    std::mt19937 rng_;

    float random_float(float min, float max);
};

}  // namespace managers
