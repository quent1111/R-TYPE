#pragma once

#include <SFML/Graphics.hpp>

#include <deque>
#include <random>
#include <vector>


class EffectsManager {
public:
    static EffectsManager& getInstance();

    EffectsManager(const EffectsManager&) = delete;
    EffectsManager& operator=(const EffectsManager&) = delete;

    void update(float dt);

    void triggerScreenShake(float intensity = 5.0f, float duration = 0.12f);
    sf::Vector2f getScreenShakeOffset() const;

    void spawnScoreParticles(sf::Vector2f from_pos, sf::Vector2f to_pos, int count = 8);

    void spawnExplosion(sf::Vector2f position, int count = 20);

    void triggerDamageFlash();
    float getDamageFlashAlpha() const;

    void addComboKill();
    void updateCombo(float dt);
    int getComboMultiplier() const;
    float getComboProgress() const;
    float getComboTimer() const;

    void triggerScoreBounce();
    float getScoreScale() const;

    void render(sf::RenderWindow& window);

    void setScorePosition(sf::Vector2f pos) { score_position_ = pos; }

private:
    EffectsManager();
    ~EffectsManager() = default;

    float shake_intensity_ = 0.0f;
    float shake_duration_ = 0.0f;
    float shake_timer_ = 0.0f;
    sf::Vector2f shake_offset_;
    std::mt19937 rng_;

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

    sf::CircleShape particle_shape_;

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

    float randomFloat(float min, float max);
};
