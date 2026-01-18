#include <gtest/gtest.h>
#include "components/logic_components.hpp"

TEST(WeaponUpgradeType, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(WeaponUpgradeType::None), 0);
    EXPECT_EQ(static_cast<uint8_t>(WeaponUpgradeType::PowerShot), 1);
    EXPECT_EQ(static_cast<uint8_t>(WeaponUpgradeType::TripleShot), 2);
}

TEST(Weapon, DefaultWeapon) {
    weapon w;
    EXPECT_FLOAT_EQ(w.fire_rate, 0.5f);
    EXPECT_FLOAT_EQ(w.time_since_shot, 0.0f);
    EXPECT_FLOAT_EQ(w.projectile_speed, 500.0f);
    EXPECT_EQ(w.damage, 10);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::None);
}

TEST(Weapon, UpdateTimer) {
    weapon w;
    w.update(0.1f);
    EXPECT_FLOAT_EQ(w.time_since_shot, 0.1f);
    
    w.update(0.15f);
    EXPECT_FLOAT_EQ(w.time_since_shot, 0.25f);
}

TEST(Weapon, ShootingCooldown) {
    weapon w(5.0f);
    
    EXPECT_FALSE(w.can_shoot());
    
    w.update(0.1f);
    EXPECT_FALSE(w.can_shoot());
    
    w.update(0.15f);
    EXPECT_TRUE(w.can_shoot());
    
    w.reset_shot_timer();
    EXPECT_FALSE(w.can_shoot());
}

TEST(Weapon, PowerShotUpgrade) {
    weapon w;
    int original_damage = w.damage;
    
    w.apply_upgrade(WeaponUpgradeType::PowerShot);
    
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::PowerShot);
    EXPECT_EQ(w.damage, 25);
    EXPECT_GT(w.damage, original_damage);
}

TEST(Weapon, TripleShotUpgrade) {
    weapon w;
    float original_fire_rate = w.fire_rate;
    
    w.apply_upgrade(WeaponUpgradeType::TripleShot);
    
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::TripleShot);
    EXPECT_FLOAT_EQ(w.fire_rate, 0.6f);
    EXPECT_GT(w.fire_rate, original_fire_rate);
}

TEST(Weapon, UpgradeOverwrite) {
    weapon w;
    
    w.apply_upgrade(WeaponUpgradeType::PowerShot);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::PowerShot);
    EXPECT_EQ(w.damage, 25);
    
    w.apply_upgrade(WeaponUpgradeType::TripleShot);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::TripleShot);
    EXPECT_FLOAT_EQ(w.fire_rate, 0.6f);
}

TEST(Weapon, CustomWeaponConfig) {
    weapon w(10.0f, 600.0f, 20, WeaponUpgradeType::PowerShot);
    
    EXPECT_FLOAT_EQ(w.fire_rate, 10.0f);
    EXPECT_FLOAT_EQ(w.projectile_speed, 600.0f);
    EXPECT_EQ(w.damage, 20);
    EXPECT_EQ(w.upgrade_type, WeaponUpgradeType::PowerShot);
}
