#include "rendering/SerpentEffectsSystem.hpp"

#include <algorithm>

namespace rendering {

SerpentEffectsSystem::SerpentEffectsSystem()
    : rng_(std::random_device{}()),
      dist_(-1.0f, 1.0f),
      scream_wave_timer_(0.0f),
      charge_particle_timer_(0.0f),
      scream_core_color_(255, 255, 200, 255),
      scream_wave_color_(255, 150, 50, 200),
      scream_outer_color_(255, 80, 20, 150),
      charge_core_color_(255, 255, 200, 255),
      charge_mid_color_(255, 150, 50, 220),
      charge_outer_color_(255, 50, 50, 180) {}

void SerpentEffectsSystem::update_scream(float dt, float head_x, float head_y, bool active) {
    for (auto& p : scream_particles_) {
        p.update(dt);
    }

    scream_particles_.erase(
        std::remove_if(scream_particles_.begin(), scream_particles_.end(),
                       [](const SerpentEffectParticle& p) { return !p.is_alive(); }),
        scream_particles_.end());

    if (!active)
        return;

    scream_wave_timer_ += dt;
    if (scream_wave_timer_ >= 0.08f) {
        scream_wave_timer_ = 0.0f;
        spawn_scream_wave(head_x, head_y);
        spawn_scream_particles(head_x, head_y);
    }
}

void SerpentEffectsSystem::spawn_scream_wave(float x, float y) {
    int ring_particles = 24;
    float base_speed = 400.0f + dist_(rng_) * 100.0f;

    for (int i = 0; i < ring_particles; ++i) {
        float angle = (static_cast<float>(i) / ring_particles) * 2.0f * 3.14159f;
        angle += dist_(rng_) * 0.1f;

        SerpentEffectParticle p;
        p.position.x = x + std::cos(angle) * 20.0f;
        p.position.y = y + std::sin(angle) * 20.0f;

        p.velocity.x = std::cos(angle) * base_speed;
        p.velocity.y = std::sin(angle) * base_speed;

        p.lifetime = 0.4f + dist_(rng_) * 0.1f;
        p.max_lifetime = p.lifetime;
        p.size = 8.0f + dist_(rng_) * 4.0f;
        p.color = scream_wave_color_;
        p.alpha = 200.0f;
        p.rotation = 0.0f;
        p.rotation_speed = dist_(rng_) * 180.0f;

        scream_particles_.push_back(p);
    }

    for (int i = 0; i < 12; ++i) {
        float angle = (static_cast<float>(i) / 12) * 2.0f * 3.14159f;

        SerpentEffectParticle p;
        p.position.x = x + std::cos(angle) * 10.0f;
        p.position.y = y + std::sin(angle) * 10.0f;

        p.velocity.x = std::cos(angle) * (base_speed * 1.2f);
        p.velocity.y = std::sin(angle) * (base_speed * 1.2f);

        p.lifetime = 0.3f;
        p.max_lifetime = p.lifetime;
        p.size = 5.0f + dist_(rng_) * 2.0f;
        p.color = scream_core_color_;
        p.alpha = 255.0f;
        p.rotation = 0.0f;
        p.rotation_speed = 0.0f;

        scream_particles_.push_back(p);
    }
}

void SerpentEffectsSystem::spawn_scream_particles(float x, float y) {
    int num_particles = 8 + static_cast<int>(dist_(rng_) * 4.0f);

    for (int i = 0; i < num_particles; ++i) {
        float angle = dist_(rng_) * 3.14159f * 2.0f;
        float speed = 150.0f + dist_(rng_) * 200.0f;

        SerpentEffectParticle p;
        p.position.x = x + dist_(rng_) * 30.0f;
        p.position.y = y + dist_(rng_) * 30.0f;

        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed;

        p.lifetime = 0.2f + dist_(rng_) * 0.15f;
        p.max_lifetime = p.lifetime;
        p.size = 3.0f + dist_(rng_) * 4.0f;
        p.color = scream_outer_color_;
        p.alpha = 180.0f;
        p.rotation = dist_(rng_) * 360.0f;
        p.rotation_speed = dist_(rng_) * 360.0f;

        scream_particles_.push_back(p);
    }

    for (int i = 0; i < 3; ++i) {
        float angle = dist_(rng_) * 3.14159f * 2.0f;
        float speed = 300.0f + dist_(rng_) * 150.0f;

        SerpentEffectParticle p;
        p.position.x = x;
        p.position.y = y;

        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed;

        p.lifetime = 0.15f + dist_(rng_) * 0.1f;
        p.max_lifetime = p.lifetime;
        p.size = 2.0f + dist_(rng_) * 2.0f;
        p.color = sf::Color(255, 200, 255, 255);
        p.alpha = 255.0f;
        p.rotation = 0.0f;
        p.rotation_speed = 0.0f;

        scream_particles_.push_back(p);
    }
}

void SerpentEffectsSystem::update_laser_charge(float dt, float tail_x, float tail_y, bool charging,
                                               float charge_progress) {
    for (auto& p : charge_particles_) {
        p.update(dt);
    }

    charge_particles_.erase(
        std::remove_if(charge_particles_.begin(), charge_particles_.end(),
                       [](const SerpentEffectParticle& p) { return !p.is_alive(); }),
        charge_particles_.end());

    if (!charging)
        return;

    charge_particle_timer_ += dt;
    float spawn_rate = 0.05f - (charge_progress * 0.03f);
    if (spawn_rate < 0.01f)
        spawn_rate = 0.01f;

    if (charge_particle_timer_ >= spawn_rate) {
        charge_particle_timer_ = 0.0f;
        spawn_charge_particles(tail_x, tail_y, charge_progress);
        spawn_charge_ring(tail_x, tail_y, charge_progress);
    }
}

void SerpentEffectsSystem::spawn_charge_particles(float x, float y, float progress) {
    int num_particles = 5 + static_cast<int>(progress * 10.0f);
    float spawn_radius = 150.0f - (progress * 80.0f);

    for (int i = 0; i < num_particles; ++i) {
        float angle = dist_(rng_) * 3.14159f * 2.0f;
        float dist = spawn_radius + dist_(rng_) * 50.0f;

        SerpentEffectParticle p;
        p.position.x = x + std::cos(angle) * dist;
        p.position.y = y + std::sin(angle) * dist;

        float dx = x - p.position.x;
        float dy = y - p.position.y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d < 1.0f)
            d = 1.0f;

        float speed = 200.0f + progress * 300.0f;
        p.velocity.x = (dx / d) * speed;
        p.velocity.y = (dy / d) * speed;

        p.lifetime = 0.3f + dist_(rng_) * 0.1f;
        p.max_lifetime = p.lifetime;
        p.size = 4.0f + progress * 4.0f + dist_(rng_) * 3.0f;

        if (progress < 0.5f) {
            p.color = charge_outer_color_;
        } else if (progress < 0.8f) {
            p.color = charge_mid_color_;
        } else {
            p.color = charge_core_color_;
        }
        p.alpha = 200.0f + progress * 55.0f;
        p.rotation = dist_(rng_) * 360.0f;
        p.rotation_speed = dist_(rng_) * 180.0f;

        charge_particles_.push_back(p);
    }
}

void SerpentEffectsSystem::spawn_charge_ring(float x, float y, float progress) {
    if (progress < 0.3f)
        return;

    int ring_particles = 16;
    float ring_radius = 30.0f + (1.0f - progress) * 40.0f;

    for (int i = 0; i < ring_particles; ++i) {
        float angle = (static_cast<float>(i) / ring_particles) * 2.0f * 3.14159f;

        SerpentEffectParticle p;
        p.position.x = x + std::cos(angle) * ring_radius;
        p.position.y = y + std::sin(angle) * ring_radius;

        p.velocity.x = -std::cos(angle) * 20.0f;
        p.velocity.y = -std::sin(angle) * 20.0f;

        p.lifetime = 0.15f;
        p.max_lifetime = p.lifetime;
        p.size = 6.0f + progress * 4.0f;
        p.color = charge_mid_color_;
        p.alpha = 150.0f + progress * 100.0f;
        p.rotation = 0.0f;
        p.rotation_speed = 0.0f;

        charge_particles_.push_back(p);
    }

    SerpentEffectParticle core;
    core.position.x = x;
    core.position.y = y;
    core.velocity.x = 0.0f;
    core.velocity.y = 0.0f;
    core.lifetime = 0.1f;
    core.max_lifetime = core.lifetime;
    core.size = 15.0f + progress * 25.0f;
    core.color = charge_core_color_;
    core.alpha = 100.0f + progress * 155.0f;
    core.rotation = 0.0f;
    core.rotation_speed = 0.0f;
    charge_particles_.push_back(core);
}

void SerpentEffectsSystem::render(sf::RenderWindow& window) {
    sf::CircleShape circle;

    for (const auto& p : scream_particles_) {
        circle.setRadius(p.size);
        circle.setOrigin(p.size, p.size);
        circle.setPosition(p.position);
        circle.setFillColor(p.color);
        window.draw(circle);
    }

    for (const auto& p : charge_particles_) {
        circle.setRadius(p.size);
        circle.setOrigin(p.size, p.size);
        circle.setPosition(p.position);
        circle.setFillColor(p.color);
        window.draw(circle);
    }
}

void SerpentEffectsSystem::clear() {
    scream_particles_.clear();
    charge_particles_.clear();
}

}  // namespace rendering
