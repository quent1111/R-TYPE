#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>

#include <chrono>
#include <vector>

struct Entity {
    uint32_t id{0};
    uint8_t type{0};

    float x{0.f}, y{0.f};
    float vx{0.f}, vy{0.f};

    float prev_x{0.f}, prev_y{0.f};
    std::chrono::steady_clock::time_point prev_time{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point curr_time{prev_time};

    sf::Sprite sprite;
    std::vector<sf::IntRect> frames;
    std::size_t current_frame_index{0};
    float frame_duration{0.1f};
    float time_accumulator{0.0f};
    bool loop{true};

    void update_animation(float dt) {
        if (frames.empty()) {
            return;
        }

        time_accumulator += dt;
        if (time_accumulator >= frame_duration) {
            time_accumulator -= frame_duration;
            current_frame_index++;

            if (current_frame_index >= frames.size()) {
                current_frame_index = loop ? 0 : frames.size() - 1;
            }

            sprite.setTextureRect(frames[current_frame_index]);
        }
    }
    Entity() = default;
};
