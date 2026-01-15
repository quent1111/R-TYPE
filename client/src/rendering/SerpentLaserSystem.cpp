#include "rendering/SerpentLaserSystem.hpp"

#include <cmath>

#include <algorithm>

namespace rendering {

SerpentLaserSystem::SerpentLaserSystem()
    : active_(false),
      spawn_timer_(0.0f),
      spawn_interval_(0.008f),
      core_color_(255, 255, 200, 255),
      mid_color_(255, 150, 50, 200),
      outer_color_(255, 50, 50, 150),
      spark_color_(255, 200, 100, 255),
      rng_(std::random_device{}()),
      dist_(-1.0f, 1.0f) {}

void SerpentLaserSystem::update(float dt, float start_x, float start_y, float angle, float length) {
    for (auto& particle : particles_) {
        particle.update(dt);
    }

    particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                    [](const SerpentLaserParticle& p) { return !p.is_alive(); }),
                     particles_.end());

    if (!active_)
        return;

    spawn_timer_ += dt;
    while (spawn_timer_ >= spawn_interval_) {
        spawn_timer_ -= spawn_interval_;
        spawn_core_particles(start_x, start_y, angle, length);
        spawn_glow_particles(start_x, start_y, angle, length);
        spawn_spark_particles(start_x, start_y, angle, length);
    }
}

void SerpentLaserSystem::spawn_core_particles(float start_x, float start_y, float angle,
                                              float length) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);

    float perp_cos = std::cos(angle + 3.14159f / 2.0f);
    float perp_sin = std::sin(angle + 3.14159f / 2.0f);

    int num_particles = static_cast<int>(length / 20.0f);

    for (int i = 0; i < num_particles; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(num_particles);
        float dist = t * length;

        float offset = dist_(rng_) * 5.0f;

        SerpentLaserParticle particle;
        particle.position.x = start_x + cos_a * dist + perp_cos * offset;
        particle.position.y = start_y + sin_a * dist + perp_sin * offset;

        particle.velocity.x = perp_cos * dist_(rng_) * 10.0f;
        particle.velocity.y = perp_sin * dist_(rng_) * 10.0f;

        particle.lifetime = 0.02f + dist_(rng_) * 0.01f;
        particle.max_lifetime = particle.lifetime;
        particle.size = 3.0f + dist_(rng_) * 2.0f;
        particle.color = core_color_;
        particle.alpha = 255.0f;

        particles_.push_back(particle);
    }
}

void SerpentLaserSystem::spawn_glow_particles(float start_x, float start_y, float angle,
                                              float length) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    float perp_cos = std::cos(angle + 3.14159f / 2.0f);
    float perp_sin = std::sin(angle + 3.14159f / 2.0f);

    int num_particles = static_cast<int>(length / 30.0f);

    for (int i = 0; i < num_particles; ++i) {
        float t = dist_(rng_) * 0.5f + 0.5f;
        float dist = t * length;
        float offset = dist_(rng_) * 15.0f;

        SerpentLaserParticle particle;
        particle.position.x = start_x + cos_a * dist + perp_cos * offset;
        particle.position.y = start_y + sin_a * dist + perp_sin * offset;

        particle.velocity.x = perp_cos * dist_(rng_) * 20.0f;
        particle.velocity.y = perp_sin * dist_(rng_) * 20.0f;

        particle.lifetime = 0.03f + dist_(rng_) * 0.02f;
        particle.max_lifetime = particle.lifetime;
        particle.size = 8.0f + dist_(rng_) * 5.0f;
        particle.color = mid_color_;
        particle.alpha = 180.0f;

        particles_.push_back(particle);
    }

    num_particles = static_cast<int>(length / 50.0f);

    for (int i = 0; i < num_particles; ++i) {
        float t = dist_(rng_) * 0.5f + 0.5f;
        float dist = t * length;
        float offset = dist_(rng_) * 25.0f;

        SerpentLaserParticle particle;
        particle.position.x = start_x + cos_a * dist + perp_cos * offset;
        particle.position.y = start_y + sin_a * dist + perp_sin * offset;

        particle.velocity.x = perp_cos * dist_(rng_) * 30.0f;
        particle.velocity.y = perp_sin * dist_(rng_) * 30.0f;

        particle.lifetime = 0.04f + dist_(rng_) * 0.02f;
        particle.max_lifetime = particle.lifetime;
        particle.size = 12.0f + dist_(rng_) * 8.0f;
        particle.color = outer_color_;
        particle.alpha = 120.0f;

        particles_.push_back(particle);
    }
}

void SerpentLaserSystem::spawn_spark_particles(float start_x, float start_y, float angle,
                                               float length) {
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);
    float perp_cos = std::cos(angle + 3.14159f / 2.0f);
    float perp_sin = std::sin(angle + 3.14159f / 2.0f);

    int num_sparks = 1 + static_cast<int>(dist_(rng_) * 2.0f);

    for (int i = 0; i < num_sparks; ++i) {
        float t = dist_(rng_) * 0.5f + 0.5f;
        float dist = t * length;
        float offset = dist_(rng_) * 10.0f;

        SerpentLaserParticle particle;
        particle.position.x = start_x + cos_a * dist + perp_cos * offset;
        particle.position.y = start_y + sin_a * dist + perp_sin * offset;

        float spark_speed = 200.0f + dist_(rng_) * 150.0f;
        float spark_angle = angle + 3.14159f / 2.0f + dist_(rng_) * 0.5f;
        particle.velocity.x = std::cos(spark_angle) * spark_speed;
        particle.velocity.y = std::sin(spark_angle) * spark_speed;

        particle.lifetime = 0.05f + dist_(rng_) * 0.03f;
        particle.max_lifetime = particle.lifetime;
        particle.size = 2.0f + dist_(rng_) * 2.0f;
        particle.color = spark_color_;
        particle.alpha = 255.0f;

        particles_.push_back(particle);
    }

    for (int i = 0; i < 1; ++i) {
        SerpentLaserParticle particle;
        particle.position.x = start_x + dist_(rng_) * 10.0f;
        particle.position.y = start_y + dist_(rng_) * 10.0f;

        float spark_speed = 80.0f + dist_(rng_) * 60.0f;
        float spark_angle = dist_(rng_) * 3.14159f * 2.0f;
        particle.velocity.x = std::cos(spark_angle) * spark_speed;
        particle.velocity.y = std::sin(spark_angle) * spark_speed;

        particle.lifetime = 0.08f + dist_(rng_) * 0.04f;
        particle.max_lifetime = particle.lifetime;
        particle.size = 4.0f + dist_(rng_) * 3.0f;
        particle.color = sf::Color(255, 100, 50, 255);
        particle.alpha = 255.0f;

        particles_.push_back(particle);
    }
}

void SerpentLaserSystem::render(sf::RenderWindow& window) {
    std::vector<SerpentLaserParticle*> sorted_particles;
    sorted_particles.reserve(particles_.size());
    for (auto& p : particles_) {
        sorted_particles.push_back(&p);
    }
    std::sort(sorted_particles.begin(), sorted_particles.end(),
              [](const SerpentLaserParticle* a, const SerpentLaserParticle* b) {
                  return a->size > b->size;
              });

    sf::CircleShape circle;

    for (const auto* particle : sorted_particles) {
        circle.setRadius(particle->size);
        circle.setOrigin(particle->size, particle->size);
        circle.setPosition(particle->position);
        circle.setFillColor(particle->color);
        window.draw(circle);
    }
}

}  // namespace rendering
