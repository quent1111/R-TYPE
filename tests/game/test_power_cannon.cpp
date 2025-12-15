#include <gtest/gtest.h>

#include "components/logic_components.hpp"

TEST(PowerCannonComponent, ActivateAndDuration) {
    power_cannon pc;
    EXPECT_FALSE(pc.is_active());

    pc.activate();
    EXPECT_TRUE(pc.is_active());
    EXPECT_GT(pc.time_remaining, 0.0f);
    EXPECT_FLOAT_EQ(pc.get_remaining_percentage(), 1.0f);

    // advance half duration
    pc.update(pc.duration / 2.0f);
    EXPECT_TRUE(pc.is_active());
    EXPECT_GT(pc.time_remaining, 0.0f);
    EXPECT_LT(pc.get_remaining_percentage(), 1.0f);

    // expire
    pc.update(pc.duration);
    EXPECT_FALSE(pc.is_active());
    EXPECT_FLOAT_EQ(pc.time_remaining, 0.0f);
    EXPECT_FLOAT_EQ(pc.get_remaining_percentage(), 0.0f);
}
