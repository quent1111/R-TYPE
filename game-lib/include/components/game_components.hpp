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
    // New: Extended mirror and rotation support
    bool mirror_x = false;     // Flip horizontally (alias for flip_horizontal)
    bool mirror_y = false;     // Flip vertically
    float rotation = 0.0f;     // Rotation in degrees

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
          grayscale(is_grayscale),
          mirror_x(flip_h),
          mirror_y(false),
          rotation(0.0f) {}
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

struct player_index_component {
    int index;

    player_index_component(int player_idx = 0)
        : index(player_idx) {}
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

// Custom attack configuration for custom level enemies
struct custom_attack_config {
    std::string pattern_type = "front";  // "front", "targeted", "spread"
    int projectile_count = 1;
    float spread_angle = 30.0f;
    bool aim_at_player = false;
    
    // Projectile sprite configuration
    std::string projectile_texture;
    int projectile_frame_width = 16;
    int projectile_frame_height = 16;
    int projectile_frame_count = 1;
    float projectile_frame_duration = 0.1f;
    float projectile_scale = 1.0f;
    bool projectile_mirror_x = false;
    bool projectile_mirror_y = false;
    float projectile_rotation = 0.0f;
};

// Component to store the ID of custom level entities (for proper identification)
struct custom_entity_id {
    std::string entity_id;  // e.g., "fairy1", "fairy2", "unicorn_boss"
    
    custom_entity_id() = default;
    explicit custom_entity_id(std::string id) : entity_id(std::move(id)) {}
};

// Note: boss_tag is already defined in logic_components.hpp

