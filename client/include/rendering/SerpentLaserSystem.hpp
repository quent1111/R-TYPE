#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <cmath>

namespace rendering {

struct SerpentLaserParticle {
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
        color.a = static_cast<sf::Uint8>(std::min(255.0f, alpha));
    }
    
    bool is_alive() const {
        return lifetime > 0.0f;
    }
};

class SerpentLaserSystem {
public:
    SerpentLaserSystem();
    ~SerpentLaserSystem() = default;
    
    // Update with position, angle (radians), and length
    void update(float dt, float start_x, float start_y, float angle, float length);
    void render(sf::RenderWindow& window);
    
    void set_active(bool active) { active_ = active; }
    bool is_active() const { return active_; }
    
private:
    void spawn_core_particles(float start_x, float start_y, float angle, float length);
    void spawn_glow_particles(float start_x, float start_y, float angle, float length);
    void spawn_spark_particles(float start_x, float start_y, float angle, float length);
    void spawn_charging_particles(float start_x, float start_y);
    
    std::vector<SerpentLaserParticle> particles_;
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;
    
    bool active_;
    float spawn_timer_;
    float spawn_interval_;
    
    // Evil laser colors - red/orange
    sf::Color core_color_;     // Bright white-yellow core
    sf::Color mid_color_;      // Orange mid
    sf::Color outer_color_;    // Red outer glow
    sf::Color spark_color_;    // Yellow-orange sparks
};

} // namespace rendering
