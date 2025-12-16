#include <gtest/gtest.h>

#include "components/logic_components.hpp"

using namespace ::testing;

TEST(LevelManager, EnemyKillProgressAndAdvance) {
    level_manager lm;

    EXPECT_EQ(lm.current_level, 1);
    EXPECT_EQ(lm.enemies_killed_this_level, 0);
    EXPECT_EQ(lm.enemies_needed_for_next_level, 1);
    EXPECT_FALSE(lm.level_completed);
    EXPECT_FALSE(lm.awaiting_upgrade_choice);

    lm.on_enemy_killed();
    EXPECT_EQ(lm.enemies_killed_this_level, 1);
    EXPECT_TRUE(lm.level_completed);
    EXPECT_TRUE(lm.awaiting_upgrade_choice);

    lm.advance_to_next_level();
    EXPECT_EQ(lm.current_level, 2);
    EXPECT_EQ(lm.enemies_killed_this_level, 0);
    EXPECT_FALSE(lm.level_completed);
    EXPECT_FALSE(lm.awaiting_upgrade_choice);
    EXPECT_EQ(lm.enemies_needed_for_next_level, 2);
}

TEST(LevelManager, ProgressPercentageAndIntroTimer) {
    level_manager lm;
    lm.enemies_needed_for_next_level = 4;
    lm.enemies_killed_this_level = 1;
    EXPECT_EQ(lm.get_progress_percentage(), 25);

    lm.level_start_delay = 1.0f;
    lm.level_start_timer = 0.0f;
    EXPECT_TRUE(lm.is_level_intro_active());
    lm.update_intro_timer(0.5f);
    EXPECT_TRUE(lm.is_level_intro_active());
    lm.update_intro_timer(1.0f);
    EXPECT_FALSE(lm.is_level_intro_active());
}
