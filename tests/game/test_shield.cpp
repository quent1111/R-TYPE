#include <gtest/gtest.h>

#include "components/logic_components.hpp"

TEST(ShieldComponent, ActivateAndExpire) {
    shield s;
    EXPECT_FALSE(s.is_active());

    s.activate();
    EXPECT_TRUE(s.is_active());
    EXPECT_GT(s.time_remaining, 0.0f);

    // Advance beyond duration
    s.update(s.duration + 1.0f);
    EXPECT_FALSE(s.is_active());
    EXPECT_FLOAT_EQ(s.time_remaining, 0.0f);
}

TEST(ShieldComponent, EnemyInRange) {
    shield s;
    s.activate();
    float player_x = 0.0f, player_y = 0.0f;

    // enemy at distance slightly less than radius
    float enemy_x = s.radius - 1.0f;
    float enemy_y = 0.0f;
    EXPECT_TRUE(s.is_enemy_in_range(enemy_x, enemy_y, player_x, player_y));

    // enemy outside radius
    enemy_x = s.radius + 1.0f;
    EXPECT_FALSE(s.is_enemy_in_range(enemy_x, enemy_y, player_x, player_y));
}
