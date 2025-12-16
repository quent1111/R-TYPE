#include <gtest/gtest.h>

#include "components/logic_components.hpp"

TEST(HealthComponent, AliveDeadAndPercentage) {
    health h(50, 100);
    EXPECT_TRUE(h.is_alive());
    EXPECT_FALSE(h.is_dead());
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.5f);

    h.current = 0;
    EXPECT_FALSE(h.is_alive());
    EXPECT_TRUE(h.is_dead());
    EXPECT_FLOAT_EQ(h.health_percentage(), 0.0f);
}
