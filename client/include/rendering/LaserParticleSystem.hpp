#pragma once

#include <SFML/Graphics.hpp>

#include <random>
#include <vector>

namespace rendering {

struct LaserParticle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;
    float max_lifetime;
    float size;
    float alpha;

    void update(float dt) {
        lifetime -= dt;
        position += velocity * dt;

        float life_ratio = lifetime / max_lifetime;
        alpha = life_ratio * 255.0f;
        color.a = static_cast<sf::Uint8>(alpha);
    }

    bool is_alive() const { return lifetime > 0.0f; }
};

class LaserParticleSystem {
public:
    LaserParticleSystem();
    ~LaserParticleSystem() = default;

    void update(float dt, float start_x, float start_y, float length);
    void render(sf::RenderWindow& window);

    void set_active(bool active) { active_ = active; }
    bool is_active() const { return active_; }

private:
    void spawn_core_particles(float start_x, float start_y, float length);
    void spawn_glow_particles(float start_x, float start_y, float length);
    void spawn_spark_particles(float start_x, float start_y, float length);

    std::vector<LaserParticle> particles_;
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;

    bool active_;
    float spawn_timer_;
    float spawn_interval_;

    sf::Color core_color_;
    sf::Color mid_color_;
    sf::Color outer_color_;
    sf::Color spark_color_;
};

}  // namespace rendering
