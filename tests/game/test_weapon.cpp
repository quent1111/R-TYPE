#include <gtest/gtest.h>

#include "components/logic_components.hpp"

TEST(WeaponComponent, FireTimerAndUpgrade) {
    weapon w;
    EXPECT_FALSE(w.can_shoot());

    w.update(0.1f);
    EXPECT_FALSE(w.can_shoot());

    w.update(1.9f);
    EXPECT_TRUE(w.can_shoot());

    w.reset_shot_timer();
    EXPECT_FALSE(w.can_shoot());
    w.apply_upgrade(WeaponUpgradeType::PowerShot);
    EXPECT_EQ(w.damage, 25);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::PowerShot);

    w.apply_upgrade(WeaponUpgradeType::TripleShot);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::TripleShot);
    EXPECT_FLOAT_EQ(w.fire_rate, 0.6f);
}
