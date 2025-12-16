#include <gtest/gtest.h>

#include "components/logic_components.hpp"

TEST(WeaponComponent, FireTimerAndUpgrade) {
    weapon w; // default fire_rate = 5.0 -> period = 0.2s
    EXPECT_FALSE(w.can_shoot());

    // advance time a bit less than required
    w.update(0.1f);
    EXPECT_FALSE(w.can_shoot());

    // advance enough
    w.update(0.2f);
    EXPECT_TRUE(w.can_shoot());

    // reset timer
    w.reset_shot_timer();
    EXPECT_FALSE(w.can_shoot());

    // apply PowerShot upgrade
    w.apply_upgrade(WeaponUpgradeType::PowerShot);
    EXPECT_EQ(w.damage, 25);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::PowerShot);

    // apply TripleShot upgrade
    w.apply_upgrade(WeaponUpgradeType::TripleShot);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::TripleShot);
    EXPECT_FLOAT_EQ(w.fire_rate, 4.0f);
}
