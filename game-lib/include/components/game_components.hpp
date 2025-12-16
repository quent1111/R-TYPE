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

    sprite_component(std::string path = "", int rect_x = 0, int rect_y = 0, int rect_w = 32,
                     int rect_h = 16, float sprite_scale = 2.0f, bool flip_h = false)
        : texture_path(std::move(path)),
          texture_rect_x(rect_x),
          texture_rect_y(rect_y),
          texture_rect_w(rect_w),
          texture_rect_h(rect_h),
          scale(sprite_scale),
          flip_horizontal(flip_h) {}
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
