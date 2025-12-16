#include <gtest/gtest.h>
#include "../../client/src/Entity.hpp"
#include <SFML/Graphics.hpp>

TEST(ClientEntity, DefaultConstruction) {
    Entity entity;

    EXPECT_EQ(entity.id, 0u);
    EXPECT_EQ(entity.type, 0u);
    EXPECT_FLOAT_EQ(entity.x, 0.0f);
    EXPECT_FLOAT_EQ(entity.y, 0.0f);
    EXPECT_FLOAT_EQ(entity.vx, 0.0f);
    EXPECT_FLOAT_EQ(entity.vy, 0.0f);
    EXPECT_EQ(entity.health, 100);
    EXPECT_EQ(entity.max_health, 100);
}

TEST(ClientEntity, HealthPercentage) {
    Entity entity;
    entity.health = 50;
    entity.max_health = 100;

    float percentage = static_cast<float>(entity.health) / static_cast<float>(entity.max_health);
    EXPECT_FLOAT_EQ(percentage, 0.5f);

    entity.health = 0;
    percentage = static_cast<float>(entity.health) / static_cast<float>(entity.max_health);
    EXPECT_FLOAT_EQ(percentage, 0.0f);
}

TEST(ClientEntity, PositionUpdate) {
    Entity entity;
    entity.x = 100.0f;
    entity.y = 200.0f;

    entity.prev_x = entity.x;
    entity.prev_y = entity.y;

    entity.x += 10.0f;
    entity.y += 20.0f;

    EXPECT_FLOAT_EQ(entity.prev_x, 100.0f);
    EXPECT_FLOAT_EQ(entity.prev_y, 200.0f);
    EXPECT_FLOAT_EQ(entity.x, 110.0f);
    EXPECT_FLOAT_EQ(entity.y, 220.0f);
}

TEST(ClientEntity, AnimationFrames) {
    Entity entity;

    entity.frames.push_back(sf::IntRect(0, 0, 32, 32));
    entity.frames.push_back(sf::IntRect(32, 0, 32, 32));
    entity.frames.push_back(sf::IntRect(64, 0, 32, 32));

    EXPECT_EQ(entity.frames.size(), 3u);
    EXPECT_EQ(entity.current_frame_index, 0u);
    EXPECT_TRUE(entity.loop);
}

TEST(ClientEntity, VelocityInterpolation) {
    Entity entity;
    entity.x = 0.0f;
    entity.y = 0.0f;
    entity.vx = 100.0f;
    entity.vy = 50.0f;

    float dt = 0.016f;

    float new_x = entity.x + entity.vx * dt;
    float new_y = entity.y + entity.vy * dt;

    EXPECT_FLOAT_EQ(new_x, 1.6f);
    EXPECT_FLOAT_EQ(new_y, 0.8f);
}
