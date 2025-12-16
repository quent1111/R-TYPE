#include <gtest/gtest.h>
#include "components/logic_components.hpp"

TEST(PowerUpType, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(PowerUpType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpType::PowerCannon), 1);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpType::Shield), 2);
}

TEST(PowerCannon, DefaultState) {
    power_cannon pc;
    EXPECT_FALSE(pc.active);
    EXPECT_FLOAT_EQ(pc.duration, 10.0f);
    EXPECT_FLOAT_EQ(pc.time_remaining, 0.0f);
    EXPECT_EQ(pc.damage, 50);
    EXPECT_FLOAT_EQ(pc.fire_rate, 3.0f);
}

TEST(PowerCannon, ActivationSequence) {
    power_cannon pc;
    
    EXPECT_FALSE(pc.is_active());
    
    pc.activate();
    
    EXPECT_TRUE(pc.is_active());
    EXPECT_FLOAT_EQ(pc.time_remaining, pc.duration);
    EXPECT_FLOAT_EQ(pc.get_remaining_percentage(), 1.0f);
}

TEST(PowerCannon, TimeDecay) {
    power_cannon pc;
    pc.activate();
    
    pc.update(5.0f);
    EXPECT_TRUE(pc.is_active());
    EXPECT_FLOAT_EQ(pc.time_remaining, 5.0f);
    EXPECT_FLOAT_EQ(pc.get_remaining_percentage(), 0.5f);
}

TEST(PowerCannon, Expiration) {
    power_cannon pc;
    pc.activate();
    
    pc.update(11.0f);
    EXPECT_FALSE(pc.is_active());
    EXPECT_FLOAT_EQ(pc.time_remaining, 0.0f);
    EXPECT_FLOAT_EQ(pc.get_remaining_percentage(), 0.0f);
}

TEST(PowerCannon, ReactivationAfterExpiry) {
    power_cannon pc;
    pc.activate();
    pc.update(11.0f);
    
    EXPECT_FALSE(pc.is_active());
    
    pc.activate();
    EXPECT_TRUE(pc.is_active());
    EXPECT_FLOAT_EQ(pc.time_remaining, pc.duration);
}

TEST(Shield, DefaultState) {
    shield s;
    EXPECT_FALSE(s.active);
    EXPECT_FLOAT_EQ(s.duration, 10.0f);
    EXPECT_FLOAT_EQ(s.time_remaining, 0.0f);
    EXPECT_FLOAT_EQ(s.radius, 80.0f);
}

TEST(Shield, RadiusDetection) {
    shield s;
    s.activate();
    
    float player_x = 100.0f, player_y = 100.0f;
    
    EXPECT_TRUE(s.is_enemy_in_range(100.0f, 150.0f, player_x, player_y));
    
    float enemy_x = player_x + s.radius;
    float enemy_y = player_y;
    EXPECT_TRUE(s.is_enemy_in_range(enemy_x, enemy_y, player_x, player_y));
    
    EXPECT_FALSE(s.is_enemy_in_range(200.0f, 200.0f, player_x, player_y));
}

TEST(Shield, InactiveShieldNoDetection) {
    shield s;
    
    float player_x = 0.0f, player_y = 0.0f;
    float enemy_x = 10.0f, enemy_y = 10.0f;
    
    EXPECT_FALSE(s.is_enemy_in_range(enemy_x, enemy_y, player_x, player_y));
}

TEST(Shield, DiagonalDistance) {
    shield s;
    s.activate();
    s.radius = 100.0f;
    
    float player_x = 0.0f, player_y = 0.0f;
    
    EXPECT_TRUE(s.is_enemy_in_range(60.0f, 60.0f, player_x, player_y));
    
    EXPECT_FALSE(s.is_enemy_in_range(80.0f, 80.0f, player_x, player_y));
}

TEST(Shield, TimeDecayAndPercentage) {
    shield s;
    s.activate();
    
    EXPECT_FLOAT_EQ(s.get_remaining_percentage(), 1.0f);
    
    s.update(2.5f);
    EXPECT_FLOAT_EQ(s.get_remaining_percentage(), 0.75f);
    
    s.update(5.0f);
    EXPECT_FLOAT_EQ(s.get_remaining_percentage(), 0.25f);
    
    s.update(3.0f);
    EXPECT_FALSE(s.is_active());
    EXPECT_FLOAT_EQ(s.get_remaining_percentage(), 0.0f);
}
