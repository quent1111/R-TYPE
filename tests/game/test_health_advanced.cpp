#include <gtest/gtest.h>
#include "components/logic_components.hpp"

TEST(Health, InitializationWithMaxHP) {
    health h(150);
    EXPECT_EQ(h.current, 150);
    EXPECT_EQ(h.maximum, 150);
}

TEST(Health, InitializationWithCurrentAndMax) {
    health h(75, 150);
    EXPECT_EQ(h.current, 75);
    EXPECT_EQ(h.maximum, 150);
}

TEST(Health, TakeDamage) {
    health h(100, 100);
    h.current -= 30;
    
    EXPECT_EQ(h.current, 70);
    EXPECT_TRUE(h.is_alive());
    EXPECT_FALSE(h.is_dead());
}

TEST(Health, FatalDamage) {
    health h(100, 100);
    h.current -= 150;
    
    EXPECT_LE(h.current, 0);
    EXPECT_FALSE(h.is_alive());
    EXPECT_TRUE(h.is_dead());
}

TEST(Health, ExactlyZeroHealth) {
    health h(50, 100);
    h.current = 0;
    
    EXPECT_EQ(h.current, 0);
    EXPECT_FALSE(h.is_alive());
    EXPECT_TRUE(h.is_dead());
}

TEST(Health, Healing) {
    health h(50, 100);
    h.current += 30;
    
    EXPECT_EQ(h.current, 80);
    EXPECT_TRUE(h.is_alive());
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.8f);
}

TEST(Health, OverHealing) {
    health h(90, 100);
    h.current += 50;
    
    if (h.current > h.maximum) h.current = h.maximum;
    
    EXPECT_EQ(h.current, 100);
    EXPECT_FLOAT_EQ(h.health_percentage(), 1.0f);
}

TEST(Health, PercentageAtVariousLevels) {
    health h(100, 100);
    
    h.current = 100;
    EXPECT_FLOAT_EQ(h.health_percentage(), 1.0f);
    
    h.current = 75;
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.75f);
    
    h.current = 50;
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.5f);
    
    h.current = 25;
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.25f);
    
    h.current = 1;
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.01f);
    
    h.current = 0;
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.0f);
}

TEST(Health, ZeroMaximumHealth) {
    health h(0, 0);
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.0f);
    EXPECT_FALSE(h.is_alive());
    EXPECT_TRUE(h.is_dead());
}

TEST(Health, DifferentMaximums) {
    health h1(50, 50);
    health h2(200, 200);
    health h3(100, 100);
    
    EXPECT_FLOAT_EQ(h1.health_percentage(), 1.0f);
    EXPECT_FLOAT_EQ(h2.health_percentage(), 1.0f);
    EXPECT_FLOAT_EQ(h3.health_percentage(), 1.0f);
    
    h1.current = 25;
    h2.current = 100;
    h3.current = 50;
    
    EXPECT_FLOAT_EQ(h1.health_percentage(), 0.5f);
    EXPECT_FLOAT_EQ(h2.health_percentage(), 0.5f);
    EXPECT_FLOAT_EQ(h3.health_percentage(), 0.5f);
}
