#include <gtest/gtest.h>
#include "components/logic_components.hpp"

TEST(LevelManagerEdge, MultipleKillsBeforeCheck) {
    level_manager lm;
    lm.enemies_needed_for_next_level = 5;
    
    for (int i = 0; i < 10; ++i) {
        lm.on_enemy_killed();
    }
    
    EXPECT_TRUE(lm.level_completed);
    EXPECT_EQ(lm.enemies_killed_this_level, 10);
}

TEST(LevelManagerEdge, ZeroEnemiesNeeded) {
    level_manager lm;
    lm.enemies_needed_for_next_level = 0;
    
    lm.on_enemy_killed();
    
    EXPECT_TRUE(lm.level_completed);
}

TEST(LevelManagerEdge, HighLevel) {
    level_manager lm;
    lm.current_level = 100;
    lm.advance_to_next_level();
    
    EXPECT_EQ(lm.current_level, 101);
    EXPECT_EQ(lm.enemies_needed_for_next_level, 101);
}

TEST(LevelManagerEdge, IntroTimerBoundary) {
    level_manager lm;
    lm.level_start_delay = 3.0f;
    lm.level_start_timer = 2.9f;
    
    EXPECT_TRUE(lm.is_level_intro_active());
    
    lm.update_intro_timer(0.2f);
    EXPECT_FALSE(lm.is_level_intro_active());
}

TEST(LevelManagerEdge, ProgressOverflow) {
    level_manager lm;
    lm.enemies_needed_for_next_level = 10;
    lm.enemies_killed_this_level = 15;
    
    int progress = lm.get_progress_percentage();
    EXPECT_GE(progress, 100);
}

TEST(HealthEdge, MassiveOverdamage) {
    health h(100, 100);
    h.current -= 10000;
    
    EXPECT_TRUE(h.is_dead());
    EXPECT_LT(h.current, 0);
}

TEST(HealthEdge, MassiveOverheal) {
    health h(100, 100);
    h.current = 10000;
    
    EXPECT_GT(h.current, h.maximum);
}

TEST(HealthEdge, NegativeMaxHealth) {
    health h(-10, -10);
    EXPECT_LE(h.maximum, 0);
}

TEST(HealthEdge, OneHP) {
    health h(1, 100);
    EXPECT_TRUE(h.is_alive());
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.01f);
}

TEST(WeaponEdge, VeryFastFireRate) {
    weapon w(1000.0f);
    w.update(0.001f);
    EXPECT_TRUE(w.can_shoot());
}

TEST(WeaponEdge, VerySlowFireRate) {
    weapon w(0.1f);
    w.update(5.0f);
    EXPECT_FALSE(w.can_shoot());
    
    w.update(6.0f);
    EXPECT_TRUE(w.can_shoot());
}

TEST(WeaponEdge, ZeroFireRate) {
    weapon w(0.0f);
    w.update(1000.0f);
}

TEST(WeaponEdge, ZeroDamage) {
    weapon w;
    w.damage = 0;
    EXPECT_EQ(w.damage, 0);
}

TEST(WeaponEdge, HighDamage) {
    weapon w;
    w.damage = 999999;
    EXPECT_EQ(w.damage, 999999);
}

TEST(ShieldEdge, VerySmallRadius) {
    shield s;
    s.activate();
    s.radius = 0.1f;
    
    EXPECT_FALSE(s.is_enemy_in_range(1.0f, 0.0f, 0.0f, 0.0f));
}

TEST(ShieldEdge, VeryLargeRadius) {
    shield s;
    s.activate();
    s.radius = 10000.0f;
    
    EXPECT_TRUE(s.is_enemy_in_range(1000.0f, 1000.0f, 0.0f, 0.0f));
}

TEST(ShieldEdge, ZeroDuration) {
    shield s;
    s.duration = 0.0f;
    s.activate();
    s.update(0.001f);
    
    EXPECT_FALSE(s.is_active());
}

TEST(ShieldEdge, ExactBoundary) {
    shield s;
    s.activate();
    s.radius = 100.0f;
    
    float player_x = 0.0f, player_y = 0.0f;
    float enemy_x = 100.0f, enemy_y = 0.0f;
    
    EXPECT_TRUE(s.is_enemy_in_range(enemy_x, enemy_y, player_x, player_y));
}

TEST(PowerCannonEdge, InstantExpiry) {
    power_cannon pc;
    pc.duration = 0.1f;
    pc.activate();
    pc.update(0.1f);
    
    EXPECT_FALSE(pc.is_active());
}

TEST(PowerCannonEdge, VeryLongDuration) {
    power_cannon pc;
    pc.duration = 1000.0f;
    pc.activate();
    pc.update(100.0f);
    
    EXPECT_TRUE(pc.is_active());
    EXPECT_GT(pc.time_remaining, 0.0f);
}

TEST(PowerCannonEdge, MultipleActivations) {
    power_cannon pc;
    
    pc.activate();
    float first_time = pc.time_remaining;
    
    pc.update(1.0f);
    
    pc.activate();
    EXPECT_FLOAT_EQ(pc.time_remaining, pc.duration);
    EXPECT_GT(pc.time_remaining, first_time - 1.0f);
}

TEST(CollisionBoxEdge, ZeroSize) {
    collision_box box(0.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(box.width, 0.0f);
    EXPECT_FLOAT_EQ(box.height, 0.0f);
}

TEST(CollisionBoxEdge, NegativeSize) {
    collision_box box(-10.0f, -20.0f, 0.0f, 0.0f);
    EXPECT_LT(box.width, 0.0f);
    EXPECT_LT(box.height, 0.0f);
}

TEST(CollisionBoxEdge, LargeOffset) {
    collision_box box(50.0f, 50.0f, 1000.0f, 2000.0f);
    EXPECT_FLOAT_EQ(box.offset_x, 1000.0f);
    EXPECT_FLOAT_EQ(box.offset_y, 2000.0f);
}

TEST(CollisionBoxEdge, NegativeOffset) {
    collision_box box(50.0f, 50.0f, -25.0f, -25.0f);
    EXPECT_FLOAT_EQ(box.offset_x, -25.0f);
    EXPECT_FLOAT_EQ(box.offset_y, -25.0f);
}

TEST(WaveManagerEdge, ZeroSpawnInterval) {
    wave_manager wm(0.0f, 3);
    EXPECT_FLOAT_EQ(wm.spawn_interval, 0.0f);
}

TEST(WaveManagerEdge, ZeroEnemiesPerWave) {
    wave_manager wm(5.0f, 0);
    EXPECT_EQ(wm.enemies_per_wave, 0);
}

TEST(WaveManagerEdge, HugeWave) {
    wave_manager wm(1.0f, 1000);
    EXPECT_EQ(wm.enemies_per_wave, 1000);
}

TEST(WaveManagerEdge, VeryFastSpawn) {
    wave_manager wm(0.01f, 5);
    wm.timer = 0.02f;
    EXPECT_GE(wm.timer, wm.spawn_interval);
}
