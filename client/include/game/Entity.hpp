#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>

#include <chrono>
#include <vector>

struct Entity {
    uint32_t id{0};
    uint8_t type{0};

    uint8_t ally_subtype{0};
    uint8_t player_index{0};

    std::string custom_entity_id;

    float x{0.f}, y{0.f};
    float vx{0.f}, vy{0.f};

    int health{100};
    int max_health{100};

    float damage_flash_timer{0.0f};
    float damage_flash_duration{0.15f};
    int prev_health{100};

    bool grayscale{false};

    float rotation{0.f};

    uint32_t attached_to{0};

    float prev_x{0.f}, prev_y{0.f};
    std::chrono::steady_clock::time_point prev_time{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point curr_time{prev_time};

    sf::Sprite sprite;
    std::vector<sf::IntRect> frames;
    std::size_t current_frame_index{0};
    float frame_duration{0.1f};
    float time_accumulator{0.0f};
    bool loop{true};
    bool ping_pong{false};
    bool forward{true};
    float pause_at_end{0.0f};
    float pause_timer{0.0f};

    void update_animation(float dt) {
        if (frames.empty()) {
            return;
        }

        if (ping_pong) {
            if (pause_at_end > 0.0f && current_frame_index == frames.size() - 1 && forward) {
                pause_timer += dt;
                if (pause_timer >= pause_at_end) {
                    pause_timer = 0.0f;
                    forward = false;
                    time_accumulator = 0.0f;
                }
                return;
            }

            time_accumulator += dt;
            if (time_accumulator >= frame_duration) {
                time_accumulator -= frame_duration;

                if (forward) {
                    current_frame_index++;
                    if (current_frame_index >= frames.size()) {
                        current_frame_index = frames.size() - 1;
                        pause_timer = 0.0f;
                    }
                } else {
                    if (current_frame_index > 0) {
                        current_frame_index--;
                    } else {
                        forward = true;
                        pause_timer = 0.0f;
                    }
                }

                sprite.setTextureRect(frames[current_frame_index]);
            }
        } else {
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
    }
    Entity() = default;
};
