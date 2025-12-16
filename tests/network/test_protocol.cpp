/**
 * @file test_protocol.cpp
 * @brief Unit tests for network protocol
 */

#include <gtest/gtest.h>
#include "../../src/Common/Opcodes.hpp"
#include "../../src/Common/BinarySerializer.hpp"

using namespace RType;

TEST(Protocol, MagicNumberValidation) {
    EXPECT_TRUE(MagicNumber::is_valid(0xB542));
    EXPECT_FALSE(MagicNumber::is_valid(0x0000));
    EXPECT_FALSE(MagicNumber::is_valid(0xFFFF));

    EXPECT_TRUE(MagicNumber::is_valid(0x42, 0xB5));
    EXPECT_FALSE(MagicNumber::is_valid(0x00, 0x00));
}

TEST(Protocol, OpCodeValues) {
    // Verify critical opcodes have expected values
    EXPECT_EQ(static_cast<uint8_t>(OpCode::Login), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::LoginAck), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::Input), 0x10);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::EntityPosition), 0x13);
    EXPECT_EQ(static_cast<uint8_t>(OpCode::PowerUpChoice), 0x34);
}

TEST(Protocol, EntityTypeValues) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Player), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Enemy), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Projectile), 0x03);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Boss), 0x08);
}

TEST(Protocol, LoginPacketStructure) {
    BinarySerializer serializer;

    // Build Login packet
    uint8_t magic1 = static_cast<uint8_t>(OpCode::MagicByte1);
    uint8_t magic2 = static_cast<uint8_t>(OpCode::MagicByte2);
    uint8_t opcode = static_cast<uint8_t>(OpCode::Login);
    std::string player_name = "TestPlayer";

    serializer << magic1 << magic2 << opcode << player_name;

    // Verify magic number
    uint8_t r_magic1, r_magic2;
    serializer >> r_magic1 >> r_magic2;
    EXPECT_TRUE(MagicNumber::is_valid(r_magic1, r_magic2));

    // Verify opcode
    uint8_t r_opcode;
    serializer >> r_opcode;
    EXPECT_EQ(r_opcode, static_cast<uint8_t>(OpCode::Login));

    // Verify player name
    std::string r_name;
    serializer >> r_name;
    EXPECT_EQ(r_name, player_name);
}

TEST(Protocol, InputPacketStructure) {
    BinarySerializer serializer;

    uint8_t magic1 = static_cast<uint8_t>(OpCode::MagicByte1);
    uint8_t magic2 = static_cast<uint8_t>(OpCode::MagicByte2);
    uint8_t opcode = static_cast<uint8_t>(OpCode::Input);
    uint8_t input_flags = 0b00001111; // Up, Down, Left, Right
    uint32_t sequence = 12345;

    serializer << magic1 << magic2 << opcode << input_flags << sequence;

    uint8_t r_magic1, r_magic2, r_opcode, r_flags;
    uint32_t r_seq;

    serializer >> r_magic1 >> r_magic2 >> r_opcode >> r_flags >> r_seq;

    EXPECT_TRUE(MagicNumber::is_valid(r_magic1, r_magic2));
    EXPECT_EQ(r_opcode, static_cast<uint8_t>(OpCode::Input));
    EXPECT_EQ(r_flags, input_flags);
    EXPECT_EQ(r_seq, sequence);
}

TEST(Protocol, PowerUpChoicePacket) {
    BinarySerializer serializer;

    uint8_t magic1 = static_cast<uint8_t>(OpCode::MagicByte1);
    uint8_t magic2 = static_cast<uint8_t>(OpCode::MagicByte2);
    uint8_t opcode = static_cast<uint8_t>(OpCode::PowerUpChoice);
    uint8_t powerup_type = 2; // Shield

    serializer << magic1 << magic2 << opcode << powerup_type;

    uint8_t r_magic1, r_magic2, r_opcode, r_powerup;
    serializer >> r_magic1 >> r_magic2 >> r_opcode >> r_powerup;

    EXPECT_TRUE(MagicNumber::is_valid(r_magic1, r_magic2));
    EXPECT_EQ(r_opcode, static_cast<uint8_t>(OpCode::PowerUpChoice));
    EXPECT_EQ(r_powerup, powerup_type);
}

