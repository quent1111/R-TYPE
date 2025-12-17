#include "managers/EffectsManager.hpp"
#include "common/Settings.hpp"

#include <cmath>

namespace managers {

EffectsManager& EffectsManager::instance() {
    static EffectsManager instance;
    return instance;
}

EffectsManager::EffectsManager() : score_position_(100.0f, 50.0f), rng_(std::random_device{}()) {
    particle_shape_.setRadius(4.0f);
    particle_shape_.setOrigin(4.0f, 4.0f);
}

void EffectsManager::update(float dt) {
    if (shake_timer_ > 0.0f) {
        shake_timer_ -= dt;
        float progress = shake_timer_ / shake_duration_;
        float current_intensity = shake_intensity_ * progress;

        shake_offset_.x = random_float(-current_intensity, current_intensity);
        shake_offset_.y = random_float(-current_intensity, current_intensity);

        if (shake_timer_ <= 0.0f) {
            shake_offset_ = sf::Vector2f(0.0f, 0.0f);
        }
    }

    if (damage_flash_timer_ > 0.0f) {
        damage_flash_timer_ -= dt;
        if (damage_flash_timer_ < 0.0f) {
            damage_flash_timer_ = 0.0f;
        }
    }

    if (combo_kills_ > 0 || combo_multiplier_ > 1) {
        combo_timer_ += dt;
        if (combo_timer_ >= combo_decay_time_) {
            combo_kills_ = 0;
            combo_multiplier_ = 1;
            combo_timer_ = 0.0f;
        }
    }

    if (score_bouncing_) {
        score_bounce_timer_ += dt;
        const float bounce_duration = 0.25f;
        if (score_bounce_timer_ < bounce_duration) {
            float t = score_bounce_timer_ / bounce_duration;
            float bounce = std::sin(t * 3.14159f) * (1.0f - t);
            score_scale_ = 1.0f + bounce * 0.3f;
        } else {
            score_scale_ = 1.0f;
            score_bouncing_ = false;
        }
    }

    for (auto it = score_particles_.begin(); it != score_particles_.end();) {
        it->lifetime -= dt;

        if (it->lifetime <= 0.0f) {
            it = score_particles_.erase(it);
            continue;
        }

        it->trail.push_front(it->position);
        if (it->trail.size() > 8) {
            it->trail.pop_back();
        }

        float progress = 1.0f - (it->lifetime / it->max_lifetime);
        float speed_mult = 1.0f + progress * 3.0f;

        sf::Vector2f to_target = it->target - it->position;
        float dist = std::sqrt(to_target.x * to_target.x + to_target.y * to_target.y);

        if (dist > 5.0f) {
            to_target /= dist;
            it->velocity = it->velocity * 0.92f + to_target * 800.0f * speed_mult * 0.08f;
            it->position += it->velocity * dt;
        }

        it->size = 8.0f * (1.0f - progress * 0.3f);
        ++it;
    }

    for (auto it = explosion_particles_.begin(); it != explosion_particles_.end();) {
        it->lifetime -= dt;

        if (it->lifetime <= 0.0f) {
            it = explosion_particles_.erase(it);
            continue;
        }

        it->velocity.y += 200.0f * dt;
        it->velocity *= 0.98f;
        it->position += it->velocity * dt;

        float progress = 1.0f - (it->lifetime / it->max_lifetime);
        it->size = 12.0f * (1.0f - progress);
        it->color.a = static_cast<sf::Uint8>(255.0f * (1.0f - progress));
        ++it;
    }
}

void EffectsManager::render(sf::RenderWindow& window) {
    for (const auto& particle : explosion_particles_) {
        particle_shape_.setRadius(particle.size);
        particle_shape_.setOrigin(particle.size, particle.size);
        particle_shape_.setPosition(particle.position);
        particle_shape_.setFillColor(particle.color);
        window.draw(particle_shape_);
    }

    for (const auto& particle : score_particles_) {
        size_t trail_index = 0;
        for (const auto& trail_pos : particle.trail) {
            float alpha = 1.0f - (static_cast<float>(trail_index) /
                                  static_cast<float>(particle.trail.size()));
            float trail_size = particle.size * alpha * 0.7f;

            particle_shape_.setRadius(trail_size);
            particle_shape_.setOrigin(trail_size, trail_size);
            particle_shape_.setPosition(trail_pos);
            particle_shape_.setFillColor(sf::Color(particle.color.r, particle.color.g,
                                                   particle.color.b,
                                                   static_cast<sf::Uint8>(alpha * 150.0f)));
            window.draw(particle_shape_);
            ++trail_index;
        }

        particle_shape_.setRadius(particle.size);
        particle_shape_.setOrigin(particle.size, particle.size);
        particle_shape_.setPosition(particle.position);
        particle_shape_.setFillColor(particle.color);
        window.draw(particle_shape_);
    }
}

void EffectsManager::trigger_screen_shake(float intensity, float duration) {
    if (!Settings::instance().screen_shake_enabled)
        return;

    shake_intensity_ = intensity;
    shake_duration_ = duration;
    shake_timer_ = duration;
}

sf::Vector2f EffectsManager::get_screen_shake_offset() const {
    return shake_offset_;
}

void EffectsManager::spawn_score_particles(sf::Vector2f from_pos, sf::Vector2f to_pos, int count) {
    for (int i = 0; i < count; ++i) {
        ScoreParticle particle;
        particle.position = from_pos;
        particle.target = to_pos;

        float angle = random_float(0.0f, 6.28318f);
        float speed = random_float(100.0f, 300.0f);
        particle.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

        particle.lifetime = random_float(0.5f, 0.9f);
        particle.max_lifetime = particle.lifetime;
        particle.size = random_float(6.0f, 10.0f);
        particle.color = sf::Color(255, 215, 0, 255);

        score_particles_.push_back(particle);
    }
}

void EffectsManager::spawn_explosion(sf::Vector2f position, int count) {
    for (int i = 0; i < count; ++i) {
        ExplosionParticle particle;
        particle.position = position;

        float angle = random_float(0.0f, 6.28318f);
        float speed = random_float(200.0f, 500.0f);
        particle.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

        particle.lifetime = random_float(0.4f, 0.8f);
        particle.max_lifetime = particle.lifetime;
        particle.size = random_float(8.0f, 16.0f);

        float color_type = random_float(0.0f, 1.0f);
        if (color_type < 0.4f) {
            particle.color =
                sf::Color(255, static_cast<sf::Uint8>(random_float(100.0f, 180.0f)), 0, 255);
        } else if (color_type < 0.7f) {
            particle.color =
                sf::Color(255, 255, static_cast<sf::Uint8>(random_float(0.0f, 100.0f)), 255);
        } else {
            particle.color =
                sf::Color(255, static_cast<sf::Uint8>(random_float(50.0f, 100.0f)), 0, 255);
        }

        explosion_particles_.push_back(particle);
    }
}

void EffectsManager::trigger_damage_flash() {
    damage_flash_timer_ = damage_flash_duration_;
}

float EffectsManager::get_damage_flash_alpha() const {
    if (damage_flash_timer_ <= 0.0f)
        return 0.0f;
    float progress = damage_flash_timer_ / damage_flash_duration_;
    return progress * 100.0f;
}

void EffectsManager::add_combo_kill() {
    combo_timer_ = 0.0f;
    combo_kills_++;

    if (combo_kills_ >= kills_per_level_ && combo_multiplier_ < 5) {
        combo_multiplier_++;
        combo_kills_ = 0;
    }
}

void EffectsManager::reset_combo() {
    combo_kills_ = 0;
    combo_multiplier_ = 1;
    combo_timer_ = 0.0f;
}

int EffectsManager::get_combo_multiplier() const {
    return combo_multiplier_;
}

float EffectsManager::get_combo_progress() const {
    float progress = static_cast<float>(combo_kills_) / static_cast<float>(kills_per_level_);
    return std::min(progress, 1.0f);
}

float EffectsManager::get_combo_timer() const {
    if (combo_kills_ == 0 && combo_multiplier_ == 1)
        return 0.0f;
    return 1.0f - (combo_timer_ / combo_decay_time_);
}

void EffectsManager::trigger_score_bounce() {
    score_bouncing_ = true;
    score_bounce_timer_ = 0.0f;
}

float EffectsManager::get_score_scale() const {
    return score_scale_;
}

void EffectsManager::set_score_position(sf::Vector2f pos) {
    score_position_ = pos;
}

float EffectsManager::random_float(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng_);
}

}  // namespace managers
