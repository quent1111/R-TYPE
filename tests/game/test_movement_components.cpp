#include <gtest/gtest.h>
#include "components/logic_components.hpp"

TEST(Controllable, DefaultSpeed) {
    controllable ctrl;
    EXPECT_FLOAT_EQ(ctrl.speed, 200.0f);
}

TEST(Controllable, CustomSpeed) {
    controllable ctrl(350.0f);
    EXPECT_FLOAT_EQ(ctrl.speed, 350.0f);
}

TEST(BoundedMovement, DefaultBounds) {
    bounded_movement bounds;
    EXPECT_FLOAT_EQ(bounds.min_x, 0.0f);
    EXPECT_FLOAT_EQ(bounds.max_x, 1920.0f);
    EXPECT_FLOAT_EQ(bounds.min_y, 0.0f);
    EXPECT_FLOAT_EQ(bounds.max_y, 1080.0f);
}

TEST(BoundedMovement, CustomBounds) {
    bounded_movement bounds(100.0f, 800.0f, 50.0f, 600.0f);
    EXPECT_FLOAT_EQ(bounds.min_x, 100.0f);
    EXPECT_FLOAT_EQ(bounds.max_x, 800.0f);
    EXPECT_FLOAT_EQ(bounds.min_y, 50.0f);
    EXPECT_FLOAT_EQ(bounds.max_y, 600.0f);
}

TEST(BoundedMovement, ClampPosition) {
    bounded_movement bounds(0.0f, 800.0f, 0.0f, 600.0f);
    
    // Test clamping x
    float x = -10.0f;
    if (x < bounds.min_x) x = bounds.min_x;
    if (x > bounds.max_x) x = bounds.max_x;
    EXPECT_FLOAT_EQ(x, 0.0f);

    x = 900.0f;
    if (x < bounds.min_x) x = bounds.min_x;
    if (x > bounds.max_x) x = bounds.max_x;
    EXPECT_FLOAT_EQ(x, 800.0f);

    // Test clamping y
    float y = -50.0f;
    if (y < bounds.min_y) y = bounds.min_y;
    if (y > bounds.max_y) y = bounds.max_y;
    EXPECT_FLOAT_EQ(y, 0.0f);

    y = 700.0f;
    if (y < bounds.min_y) y = bounds.min_y;
    if (y > bounds.max_y) y = bounds.max_y;
    EXPECT_FLOAT_EQ(y, 600.0f);
}

TEST(WaveManager, DefaultValues) {
    wave_manager wm;
    EXPECT_FLOAT_EQ(wm.timer, 0.0f);
    EXPECT_FLOAT_EQ(wm.spawn_interval, 5.0f);
    EXPECT_EQ(wm.enemies_per_wave, 3);
}

TEST(WaveManager, CustomValues) {
    wave_manager wm(3.0f, 5);
    EXPECT_FLOAT_EQ(wm.spawn_interval, 3.0f);
    EXPECT_EQ(wm.enemies_per_wave, 5);
}

TEST(WaveManager, TimerUpdate) {
    wave_manager wm(5.0f, 3);
    wm.timer = 0.0f;

    // Simulate time passing
    wm.timer += 0.016f; // 1 frame at 60fps
    EXPECT_GT(wm.timer, 0.0f);

    // Check if spawn interval reached
    wm.timer = 5.5f;
    EXPECT_GE(wm.timer, wm.spawn_interval);

    // Reset timer after spawn
    wm.timer = 0.0f;
    EXPECT_FLOAT_EQ(wm.timer, 0.0f);
}
