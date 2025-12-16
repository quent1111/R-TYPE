#include <gtest/gtest.h>
#include "../../src/Common/Opcodes.hpp"

using namespace RType;

TEST(EntityTypeTests, PlayerValue) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Player), 0x01);
}

TEST(EntityTypeTests, EnemyValue) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Enemy), 0x02);
}

TEST(EntityTypeTests, ProjectileValue) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Projectile), 0x03);
}

TEST(EntityTypeTests, PowerupValue) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Powerup), 0x04);
}

TEST(EntityTypeTests, ObstacleValue) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Obstacle), 0x05);
}

TEST(EntityTypeTests, Enemy2Value) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Enemy2), 0x06);
}

TEST(EntityTypeTests, BossValue) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Boss), 0x08);
}

TEST(EntityTypeTests, AllUnique) {
    std::set<uint8_t> types;
    types.insert(static_cast<uint8_t>(EntityType::Player));
    types.insert(static_cast<uint8_t>(EntityType::Enemy));
    types.insert(static_cast<uint8_t>(EntityType::Projectile));
    types.insert(static_cast<uint8_t>(EntityType::Powerup));
    types.insert(static_cast<uint8_t>(EntityType::Obstacle));
    types.insert(static_cast<uint8_t>(EntityType::Enemy2));
    types.insert(static_cast<uint8_t>(EntityType::Boss));
    
    EXPECT_EQ(types.size(), 7);
}

TEST(OpCodeTests, LoginValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::Login), 0x01);
}

TEST(OpCodeTests, LoginAckValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::LoginAck), 0x02);
}

TEST(OpCodeTests, InputValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::Input), 0x10);
}

TEST(OpCodeTests, EntitySpawnValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::EntitySpawn), 0x11);
}

TEST(OpCodeTests, EntityDestroyValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::EntityDestroy), 0x12);
}

TEST(OpCodeTests, EntityPositionValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::EntityPosition), 0x13);
}

TEST(OpCodeTests, PlayerReadyValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::PlayerReady), 0x20);
}

TEST(OpCodeTests, LobbyStatusValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::LobbyStatus), 0x21);
}

TEST(OpCodeTests, StartGameValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::StartGame), 0x22);
}

TEST(OpCodeTests, LevelStartValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::LevelStart), 0x30);
}

TEST(OpCodeTests, LevelCompleteValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::LevelComplete), 0x31);
}

TEST(OpCodeTests, WeaponUpgradeChoiceValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::WeaponUpgradeChoice), 0x32);
}

TEST(OpCodeTests, LevelProgressValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::LevelProgress), 0x33);
}

TEST(OpCodeTests, PowerUpChoiceValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::PowerUpChoice), 0x34);
}

TEST(OpCodeTests, PowerUpActivateValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::PowerUpActivate), 0x35);
}

TEST(OpCodeTests, PowerUpStatusValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::PowerUpStatus), 0x36);
}

TEST(OpCodeTests, GameOverValue) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::GameOver), 0x40);
}

TEST(OpCodeTests, MagicByte1Value) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::MagicByte1), 0x42);
}

TEST(OpCodeTests, MagicByte2Value) {
    EXPECT_EQ(static_cast<uint8_t>(OpCode::MagicByte2), 0xB5);
}

TEST(MagicNumberTests, VALUE) {
    EXPECT_EQ(MagicNumber::VALUE, 0xB542);
}

TEST(MagicNumberTests, BYTE1) {
    EXPECT_EQ(MagicNumber::BYTE1, 0x42);
}

TEST(MagicNumberTests, BYTE2) {
    EXPECT_EQ(MagicNumber::BYTE2, 0xB5);
}

TEST(MagicNumberTests, ValidValue) {
    EXPECT_TRUE(MagicNumber::is_valid(0xB542));
}

TEST(MagicNumberTests, ValidBytes) {
    EXPECT_TRUE(MagicNumber::is_valid(0x42, 0xB5));
}

TEST(MagicNumberTests, InvalidValue) {
    EXPECT_FALSE(MagicNumber::is_valid(0x0000));
    EXPECT_FALSE(MagicNumber::is_valid(0xFFFF));
    EXPECT_FALSE(MagicNumber::is_valid(0x1234));
}

TEST(MagicNumberTests, InvalidBytes) {
    EXPECT_FALSE(MagicNumber::is_valid(0x00, 0x00));
    EXPECT_FALSE(MagicNumber::is_valid(0xFF, 0xFF));
    EXPECT_FALSE(MagicNumber::is_valid(0x42, 0x00));
    EXPECT_FALSE(MagicNumber::is_valid(0x00, 0xB5));
}
