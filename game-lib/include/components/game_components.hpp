#pragma once

#include "logic_components.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include "../../../engine/ecs/components.hpp"


struct sprite_component {
    std::string texture_path;
    int texture_rect_x;
    int texture_rect_y;
    int texture_rect_w;
    int texture_rect_h;
    float scale;
    bool flip_horizontal;
    bool visible;
    bool grayscale;

    sprite_component(std::string path = "", int rect_x = 0, int rect_y = 0, int rect_w = 32,
                     int rect_h = 16, float sprite_scale = 2.0f, bool flip_h = false, bool is_visible = true,
                     bool is_grayscale = false)
        : texture_path(std::move(path)),
          texture_rect_x(rect_x),
          texture_rect_y(rect_y),
          texture_rect_w(rect_w),
          texture_rect_h(rect_h),
          scale(sprite_scale),
          flip_horizontal(flip_h),
          visible(is_visible),
          grayscale(is_grayscale) {}
};

struct animation_component {
    std::vector<sf::IntRect> frames;
    size_t current_frame;
    float frame_duration;
    float time_accumulator;
    bool loop;

    animation_component(std::vector<sf::IntRect> anim_frames = {}, float duration = 0.1f,
                        bool should_loop = true)
        : frames(std::move(anim_frames)),
          current_frame(0),
          frame_duration(duration),
          time_accumulator(0.0f),
          loop(should_loop) {}

    void update(float dt) {
        if (frames.empty()) return;

        time_accumulator += dt;
        if (time_accumulator >= frame_duration) {
            time_accumulator -= frame_duration;
            current_frame++;

            if (current_frame >= frames.size()) {
                current_frame = loop ? 0 : frames.size() - 1;
            }
        }
    }

    [[nodiscard]] sf::IntRect get_current_frame() const {
        if (frames.empty()) {
            return sf::IntRect{0, 0, 32, 16};
        }
        return frames[current_frame];
    }
};

struct damage_flash_component {
    float timer;
    float duration;
    bool active;

    damage_flash_component(float flash_duration = 0.15f)
        : timer(0.0f), duration(flash_duration), active(false) {}

    void trigger() {
        active = true;
        timer = duration;
    }

    void update(float dt) {
        if (active) {
            timer -= dt;
            if (timer <= 0.0f) {
                active = false;
                timer = 0.0f;
            }
        }
    }

    [[nodiscard]] float get_alpha() const {
        if (!active) return 0.0f;
        return (timer / duration) * 255.0f;
    }
};

struct homing_component {
    float speed;
    float turn_rate;
    
    homing_component(float homing_speed = 250.0f, float homing_turn_rate = 3.0f)
        : speed(homing_speed), turn_rate(homing_turn_rate) {}
};

struct laser_damage_immunity {
    float immunity_timer;
    float immunity_duration;
    
    laser_damage_immunity(float duration = 0.1f)
        : immunity_timer(0.0f), immunity_duration(duration) {}
    
    bool is_immune() const {
        return immunity_timer > 0.0f;
    }
    
    void trigger() {
        immunity_timer = immunity_duration;
    }
    
    void update(float dt) {
        if (immunity_timer > 0.0f) {
            immunity_timer -= dt;
            if (immunity_timer < 0.0f) immunity_timer = 0.0f;
        }
    }
};

struct explosive_projectile {
    float lifetime;
    float max_lifetime;
    float explosion_radius;
    int explosion_damage;
    bool has_exploded;

    explosive_projectile(float life = 2.0f, float radius = 80.0f, int damage = 40)
        : lifetime(life), max_lifetime(life), explosion_radius(radius),
          explosion_damage(damage), has_exploded(false) {}

    void update(float dt) {
        if (!has_exploded) {
            lifetime -= dt;
        }
    }

    bool should_explode() const {
        return !has_exploded && lifetime <= 0.0f;
    }
};

