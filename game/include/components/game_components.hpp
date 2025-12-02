#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include "../../../engine/ecs/components.hpp"

struct controllable {
    float speed;
    constexpr explicit controllable(float move_speed = 200.0f) noexcept : speed(move_speed) {}
};

struct weapon {
    float fire_rate;
    float time_since_shot;
    float projectile_speed;
    int damage;

    constexpr weapon(float rate = 5.0f, float proj_speed = 500.0f, int dmg = 10) noexcept
        : fire_rate(rate), time_since_shot(0.0f), projectile_speed(proj_speed), damage(dmg) {}

    constexpr void update(float dt) noexcept { time_since_shot += dt; }

    [[nodiscard]] constexpr bool can_shoot() const noexcept {
        return time_since_shot >= (1.0f / fire_rate);
    }

    constexpr void reset_shot_timer() noexcept { time_since_shot = 0.0f; }
};

struct sprite_component {
    std::string texture_path;
    int texture_rect_x;
    int texture_rect_y;
    int texture_rect_w;
    int texture_rect_h;
    float scale;

    sprite_component(std::string path = "", int rect_x = 0, int rect_y = 0, int rect_w = 32,
                     int rect_h = 16, float sprite_scale = 2.0f)
        : texture_path(std::move(path)),
          texture_rect_x(rect_x),
          texture_rect_y(rect_y),
          texture_rect_w(rect_w),
          texture_rect_h(rect_h),
          scale(sprite_scale) {}
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

struct explosion_tag {
    float lifetime;
    float elapsed;

    constexpr explosion_tag(float life = 0.5f) noexcept
        : lifetime(life), elapsed(0.0f) {}
};

struct bounded_movement {
    float min_x, max_x;
    float min_y, max_y;

    constexpr bounded_movement(float minx = 0.0f, float maxx = 1920.0f, float miny = 0.0f,
                                float maxy = 1080.0f) noexcept
        : min_x(minx), max_x(maxx), min_y(miny), max_y(maxy) {}
};
