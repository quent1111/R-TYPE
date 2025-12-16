#include <gtest/gtest.h>
#include <cmath>
#include "../../client/src/Entity.hpp"

TEST(ClientEntityAdvanced, VelocityMagnitude) {
    Entity entity;
    entity.vx = 3.0f;
    entity.vy = 4.0f;
    
    float magnitude = std::sqrt(entity.vx * entity.vx + entity.vy * entity.vy);
    EXPECT_FLOAT_EQ(magnitude, 5.0f);
}

TEST(ClientEntityAdvanced, NormalizeVelocity) {
    Entity entity;
    entity.vx = 3.0f;
    entity.vy = 4.0f;
    
    float magnitude = std::sqrt(entity.vx * entity.vx + entity.vy * entity.vy);
    float normalized_vx = entity.vx / magnitude;
    float normalized_vy = entity.vy / magnitude;
    
    EXPECT_FLOAT_EQ(normalized_vx, 0.6f);
    EXPECT_FLOAT_EQ(normalized_vy, 0.8f);
}

TEST(ClientEntityAdvanced, ZeroVelocity) {
    Entity entity;
    entity.vx = 0.0f;
    entity.vy = 0.0f;
    
    float magnitude = std::sqrt(entity.vx * entity.vx + entity.vy * entity.vy);
    EXPECT_FLOAT_EQ(magnitude, 0.0f);
}

TEST(ClientEntityAdvanced, HighVelocity) {
    Entity entity;
    entity.vx = 1000.0f;
    entity.vy = 1000.0f;
    
    float magnitude = std::sqrt(entity.vx * entity.vx + entity.vy * entity.vy);
    EXPECT_GT(magnitude, 1000.0f);
}

TEST(ClientEntityAdvanced, NegativeVelocity) {
    Entity entity;
    entity.vx = -50.0f;
    entity.vy = -50.0f;
    
    EXPECT_LT(entity.vx, 0.0f);
    EXPECT_LT(entity.vy, 0.0f);
}

TEST(ClientEntityAdvanced, PositionDelta) {
    Entity entity;
    entity.x = 100.0f;
    entity.y = 200.0f;
    entity.prev_x = 90.0f;
    entity.prev_y = 180.0f;
    
    float dx = entity.x - entity.prev_x;
    float dy = entity.y - entity.prev_y;
    
    EXPECT_FLOAT_EQ(dx, 10.0f);
    EXPECT_FLOAT_EQ(dy, 20.0f);
}

TEST(ClientEntityAdvanced, DistanceCalculation) {
    Entity e1, e2;
    e1.x = 0.0f;
    e1.y = 0.0f;
    e2.x = 30.0f;
    e2.y = 40.0f;
    
    float dx = e2.x - e1.x;
    float dy = e2.y - e1.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    EXPECT_FLOAT_EQ(distance, 50.0f);
}

TEST(ClientEntityAdvanced, EntityTypes) {
    Entity player, enemy, projectile;
    
    player.type = 0x01;
    enemy.type = 0x02;
    projectile.type = 0x03;
    
    EXPECT_EQ(player.type, 0x01);
    EXPECT_EQ(enemy.type, 0x02);
    EXPECT_EQ(projectile.type, 0x03);
    EXPECT_NE(player.type, enemy.type);
}

TEST(ClientEntityAdvanced, MultipleIDs) {
    Entity e1, e2, e3;
    e1.id = 1;
    e2.id = 2;
    e3.id = 3;
    
    EXPECT_NE(e1.id, e2.id);
    EXPECT_NE(e2.id, e3.id);
    EXPECT_NE(e1.id, e3.id);
}

TEST(ClientEntityAdvanced, HealthBoundaries) {
    Entity entity;
    entity.max_health = 100;
    
    entity.health = 0;
    EXPECT_EQ(entity.health, 0);
    
    entity.health = 50;
    EXPECT_EQ(entity.health, 50);
    
    entity.health = 100;
    EXPECT_EQ(entity.health, 100);
    
    entity.health = 150; // Over max (should be clamped in game logic)
    EXPECT_GT(entity.health, entity.max_health);
}

TEST(ClientEntityAnimation, PingPongMode) {
    Entity entity;
    entity.ping_pong = true;
    entity.forward = true;
    
    EXPECT_TRUE(entity.ping_pong);
    EXPECT_TRUE(entity.forward);
}

TEST(ClientEntityAnimation, LoopMode) {
    Entity entity;
    entity.loop = true;
    
    EXPECT_TRUE(entity.loop);
}

TEST(ClientEntityAnimation, FrameDuration) {
    Entity entity;
    entity.frame_duration = 0.1f;
    
    EXPECT_FLOAT_EQ(entity.frame_duration, 0.1f);
}

TEST(ClientEntityAnimation, TimeAccumulator) {
    Entity entity;
    entity.time_accumulator = 0.0f;
    entity.time_accumulator += 0.05f;
    
    EXPECT_FLOAT_EQ(entity.time_accumulator, 0.05f);
}

TEST(ClientEntityAnimation, PauseTimer) {
    Entity entity;
    entity.pause_at_end = 0.5f;
    entity.pause_timer = 0.0f;
    
    entity.pause_timer += 0.1f;
    EXPECT_FLOAT_EQ(entity.pause_timer, 0.1f);
    EXPECT_LT(entity.pause_timer, entity.pause_at_end);
}
