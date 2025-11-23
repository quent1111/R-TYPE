#pragma once

#include <SFML/Graphics.hpp>

// Component: 2D position of an entity
struct position {
    float x;
    float y;

    constexpr position(float x = 0.0f, float y = 0.0f) noexcept : x(x), y(y) {}
};

// Component: 2D velocity of an entity
struct velocity {
    float vx;
    float vy;

    constexpr velocity(float vx = 0.0f, float vy = 0.0f) noexcept : vx(vx), vy(vy) {}
};

// Component: Makes an entity drawable with SFML
struct drawable {
    sf::RectangleShape shape;
    explicit drawable(sf::Vector2f size = sf::Vector2f(50.0f, 50.0f),
                      sf::Color color = sf::Color::White) {
        shape.setSize(size);
        shape.setFillColor(color);
    }
};

// Component: Marks an entity as controllable by keyboard
struct controllable {
    float speed;  // Movement speed when keys are pressed
    constexpr explicit controllable(float speed = 200.0f) noexcept : speed(speed) {}
};

// Component: 2D acceleration for physics simulation
struct acceleration {
    float ax;
    float ay;
    float max_speed;  // Maximum speed limit
    float friction;   // Deceleration when no input (0.0 = no friction, 1.0 = instant stop)

    constexpr acceleration(float ax = 0.0f, float ay = 0.0f, float max_speed = 300.0f,
                           float friction = 0.95f) noexcept
        : ax(ax), ay(ay), max_speed(max_speed), friction(friction) {}
};

// Component: Makes an entity loop around screen boundaries
struct looping {
    float screen_width;
    float screen_height;

    constexpr looping(float width = 800.0f, float height = 600.0f) noexcept
        : screen_width(width), screen_height(height) {}
};
