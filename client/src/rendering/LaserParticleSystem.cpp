#include "rendering/LaserParticleSystem.hpp"

#include <cmath>

namespace rendering {

LaserParticleSystem::LaserParticleSystem()
    : rng_(std::random_device{}()),
      dist_(0.0f, 1.0f),
      active_(false),
      spawn_timer_(0.0f),
      spawn_interval_(0.008f),
      core_color_(255, 255, 255, 255),
      mid_color_(120, 200, 255, 200),
      outer_color_(40, 120, 255, 150),
      spark_color_(0, 255, 255, 255) {
    particles_.reserve(2000);
}

void LaserParticleSystem::update(float dt, float start_x, float start_y, float length) {
    if (!active_) {
        particles_.clear();
        return;
    }

    spawn_timer_ += dt;
    if (spawn_timer_ >= spawn_interval_) {
        spawn_timer_ = 0.0f;

        spawn_core_particles(start_x, start_y, length);
        spawn_glow_particles(start_x, start_y, length);
        spawn_spark_particles(start_x, start_y, length);
    }

    for (auto& particle : particles_) {
        particle.update(dt);
    }

    particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                    [](const LaserParticle& p) { return !p.is_alive(); }),
                     particles_.end());
}

void LaserParticleSystem::spawn_core_particles(float start_x, float start_y, float length) {
    int num_core = 30;
    for (int i = 0; i < num_core; ++i) {
        LaserParticle particle;

        float t = dist_(rng_);
        particle.position.x = start_x + length * t;
        particle.position.y = start_y + (dist_(rng_) - 0.5f) * 20.0f;

        particle.velocity.x = 50.0f + dist_(rng_) * 30.0f;
        particle.velocity.y = (dist_(rng_) - 0.5f) * 15.0f;

        particle.color = core_color_;
        particle.size = 8.0f + dist_(rng_) * 6.0f;
        particle.lifetime = 0.15f + dist_(rng_) * 0.1f;
        particle.max_lifetime = particle.lifetime;
        particle.alpha = 255.0f;

        particles_.push_back(particle);
    }
}

void LaserParticleSystem::spawn_glow_particles(float start_x, float start_y, float length) {
    int num_glow = 40;
    for (int i = 0; i < num_glow; ++i) {
        LaserParticle particle;

        float t = dist_(rng_);
        particle.position.x = start_x + length * t;

        float vertical_spread = 40.0f;
        particle.position.y = start_y + (dist_(rng_) - 0.5f) * vertical_spread;

        particle.velocity.x = 30.0f + dist_(rng_) * 40.0f;
        particle.velocity.y = (dist_(rng_) - 0.5f) * 25.0f;

        float dist_from_center = std::abs(particle.position.y - start_y);
        float blend = std::min(1.0f, dist_from_center / vertical_spread);

        particle.color.r =
            static_cast<sf::Uint8>(mid_color_.r * (1.0f - blend) + outer_color_.r * blend);
        particle.color.g =
            static_cast<sf::Uint8>(mid_color_.g * (1.0f - blend) + outer_color_.g * blend);
        particle.color.b =
            static_cast<sf::Uint8>(mid_color_.b * (1.0f - blend) + outer_color_.b * blend);
        particle.color.a =
            static_cast<sf::Uint8>(mid_color_.a * (1.0f - blend) + outer_color_.a * blend);

        particle.size = 5.0f + dist_(rng_) * 4.0f;
        particle.lifetime = 0.2f + dist_(rng_) * 0.15f;
        particle.max_lifetime = particle.lifetime;
        particle.alpha = particle.color.a;

        particles_.push_back(particle);
    }
}

void LaserParticleSystem::spawn_spark_particles(float start_x, float start_y, float length) {
    int num_sparks = 15;
    for (int i = 0; i < num_sparks; ++i) {
        LaserParticle particle;

        float t = dist_(rng_);
        particle.position.x = start_x + length * t;
        particle.position.y = start_y + (dist_(rng_) - 0.5f) * 35.0f;

        float angle = dist_(rng_) * 6.28318f;
        float speed = 80.0f + dist_(rng_) * 60.0f;
        particle.velocity.x = std::cos(angle) * speed;
        particle.velocity.y = std::sin(angle) * speed;

        particle.color = spark_color_;
        particle.size = 3.0f + dist_(rng_) * 2.0f;
        particle.lifetime = 0.1f + dist_(rng_) * 0.08f;
        particle.max_lifetime = particle.lifetime;
        particle.alpha = 255.0f;

        particles_.push_back(particle);
    }

    if (dist_(rng_) > 0.6f) {
        LaserParticle edge_spark;
        edge_spark.position.x = start_x + length * dist_(rng_);
        edge_spark.position.y = start_y + (dist_(rng_) > 0.5f ? 45.0f : -45.0f);

        edge_spark.velocity.x = (dist_(rng_) - 0.5f) * 100.0f;
        edge_spark.velocity.y = (dist_(rng_) - 0.5f) * 100.0f;

        edge_spark.color = spark_color_;
        edge_spark.size = 3.0f;
        edge_spark.lifetime = 0.12f;
        edge_spark.max_lifetime = edge_spark.lifetime;
        edge_spark.alpha = 255.0f;

        particles_.push_back(edge_spark);
    }
}

void LaserParticleSystem::render(sf::RenderWindow& window) {
    if (!active_ || particles_.empty()) {
        return;
    }

    sf::RenderStates states;
    states.blendMode = sf::BlendAdd;

    for (const auto& particle : particles_) {
        sf::CircleShape circle(particle.size);
        circle.setFillColor(particle.color);
        circle.setOrigin(particle.size, particle.size);
        circle.setPosition(particle.position);

        window.draw(circle, states);

        if (particle.size > 5.0f) {
            sf::CircleShape glow(particle.size * 1.5f);
            sf::Color glow_color = particle.color;
            glow_color.a = particle.color.a / 3;
            glow.setFillColor(glow_color);
            glow.setOrigin(particle.size * 1.5f, particle.size * 1.5f);
            glow.setPosition(particle.position);

            window.draw(glow, states);
        }
    }
}

}  // namespace rendering
