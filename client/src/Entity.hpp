#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <chrono>

struct Entity {
    uint32_t id{0};
    uint8_t type{0};

    float x{0.f}, y{0.f};
    float vx{0.f}, vy{0.f};

    float prev_x{0.f}, prev_y{0.f};
    std::chrono::steady_clock::time_point prev_time{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point curr_time{prev_time};

    sf::RectangleShape shape{sf::Vector2f(30.f, 30.f)};

    Entity() = default;
};