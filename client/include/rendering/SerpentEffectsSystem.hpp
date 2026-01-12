#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <cmath>

namespace rendering {

struct SerpentEffectParticle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;
    float max_lifetime;
    float size;
    float alpha;
    float rotation;
    float rotation_speed;
    
    void update(float dt) {
        lifetime -= dt;
        position += velocity * dt;
        rotation += rotation_speed * dt;
        
        float life_ratio = lifetime / max_lifetime;
        alpha = life_ratio * 255.0f;
        color.a = static_cast<sf::Uint8>(std::min(255.0f, std::max(0.0f, alpha)));
    }
    
    bool is_alive() const {
        return lifetime > 0.0f;
    }
};

class SerpentEffectsSystem {
public:
    SerpentEffectsSystem();
    ~SerpentEffectsSystem() = default;
    
    void update_scream(float dt, float head_x, float head_y, bool active);
    
    void update_laser_charge(float dt, float tail_x, float tail_y, bool charging, float charge_progress);
    
    void render(sf::RenderWindow& window);
    void clear();
    
private:
    void spawn_scream_wave(float x, float y);
    void spawn_scream_particles(float x, float y);
    void spawn_charge_particles(float x, float y, float progress);
    void spawn_charge_ring(float x, float y, float progress);
    
    std::vector<SerpentEffectParticle> scream_particles_;
    std::vector<SerpentEffectParticle> charge_particles_;
    
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;
    
    float scream_wave_timer_;
    float charge_particle_timer_;
    
    sf::Color scream_core_color_;
    sf::Color scream_wave_color_;
    sf::Color scream_outer_color_;
    
    sf::Color charge_core_color_;
    sf::Color charge_mid_color_;
    sf::Color charge_outer_color_;
};

} // namespace rendering
