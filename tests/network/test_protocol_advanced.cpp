#include <gtest/gtest.h>
#include "../../src/Common/Opcodes.hpp"
#include "../../src/Common/BinarySerializer.hpp"

using namespace RType;

TEST(ProtocolAdvanced, AllOpCodes) {
    // Verify all opcodes are unique
    std::set<uint8_t> opcodes;
    opcodes.insert(static_cast<uint8_t>(OpCode::Login));
    opcodes.insert(static_cast<uint8_t>(OpCode::LoginAck));
    opcodes.insert(static_cast<uint8_t>(OpCode::Input));
    opcodes.insert(static_cast<uint8_t>(OpCode::EntitySpawn));
    opcodes.insert(static_cast<uint8_t>(OpCode::EntityDestroy));
    opcodes.insert(static_cast<uint8_t>(OpCode::EntityPosition));
    opcodes.insert(static_cast<uint8_t>(OpCode::PlayerReady));
    opcodes.insert(static_cast<uint8_t>(OpCode::LobbyStatus));
    opcodes.insert(static_cast<uint8_t>(OpCode::StartGame));
    opcodes.insert(static_cast<uint8_t>(OpCode::LevelStart));
    opcodes.insert(static_cast<uint8_t>(OpCode::LevelComplete));
    opcodes.insert(static_cast<uint8_t>(OpCode::WeaponUpgradeChoice));
    opcodes.insert(static_cast<uint8_t>(OpCode::LevelProgress));
    opcodes.insert(static_cast<uint8_t>(OpCode::PowerUpChoice));
    opcodes.insert(static_cast<uint8_t>(OpCode::PowerUpActivate));
    opcodes.insert(static_cast<uint8_t>(OpCode::PowerUpStatus));
    opcodes.insert(static_cast<uint8_t>(OpCode::GameOver));
    
    EXPECT_EQ(opcodes.size(), 17); // All opcodes are unique
}

TEST(ProtocolAdvanced, EntitySpawnPacket) {
    BinarySerializer serializer;
    
    uint8_t magic1 = static_cast<uint8_t>(OpCode::MagicByte1);
    uint8_t magic2 = static_cast<uint8_t>(OpCode::MagicByte2);
    uint8_t opcode = static_cast<uint8_t>(OpCode::EntitySpawn);
    uint32_t entity_id = 999;
    uint8_t entity_type = static_cast<uint8_t>(EntityType::Enemy);
    float x = 500.0f, y = 300.0f;
    
    serializer << magic1 << magic2 << opcode 
               << entity_id << entity_type << x << y;
    
    uint8_t r_magic1, r_magic2, r_opcode, r_type;
    uint32_t r_id;
    float r_x, r_y;
    
    serializer >> r_magic1 >> r_magic2 >> r_opcode 
               >> r_id >> r_type >> r_x >> r_y;
    
    EXPECT_TRUE(MagicNumber::is_valid(r_magic1, r_magic2));
    EXPECT_EQ(r_opcode, static_cast<uint8_t>(OpCode::EntitySpawn));
    EXPECT_EQ(r_id, entity_id);
    EXPECT_EQ(r_type, static_cast<uint8_t>(EntityType::Enemy));
    EXPECT_FLOAT_EQ(r_x, x);
    EXPECT_FLOAT_EQ(r_y, y);
}

TEST(ProtocolAdvanced, EntityDestroyPacket) {
    BinarySerializer serializer;
    
    serializer << static_cast<uint8_t>(OpCode::MagicByte1)
               << static_cast<uint8_t>(OpCode::MagicByte2)
               << static_cast<uint8_t>(OpCode::EntityDestroy)
               << static_cast<uint32_t>(456);
    
    uint8_t magic1, magic2, opcode;
    uint32_t entity_id;
    
    serializer >> magic1 >> magic2 >> opcode >> entity_id;
    
    EXPECT_TRUE(MagicNumber::is_valid(magic1, magic2));
    EXPECT_EQ(opcode, static_cast<uint8_t>(OpCode::EntityDestroy));
    EXPECT_EQ(entity_id, 456u);
}

TEST(ProtocolAdvanced, LevelStartPacket) {
    BinarySerializer serializer;
    
    serializer << static_cast<uint8_t>(OpCode::MagicByte1)
               << static_cast<uint8_t>(OpCode::MagicByte2)
               << static_cast<uint8_t>(OpCode::LevelStart)
               << static_cast<uint32_t>(5); // level number
    
    uint8_t magic1, magic2, opcode;
    uint32_t level;
    
    serializer >> magic1 >> magic2 >> opcode >> level;
    
    EXPECT_EQ(opcode, static_cast<uint8_t>(OpCode::LevelStart));
    EXPECT_EQ(level, 5u);
}

TEST(ProtocolAdvanced, LevelCompletePacket) {
    BinarySerializer serializer;
    
    serializer << static_cast<uint8_t>(OpCode::MagicByte1)
               << static_cast<uint8_t>(OpCode::MagicByte2)
               << static_cast<uint8_t>(OpCode::LevelComplete)
               << static_cast<uint32_t>(3); // level completed
    
    uint8_t opcode;
    uint8_t magic1, magic2;
    serializer.reset_read_position();
    serializer >> magic1 >> magic2; // skip magic
    serializer >> opcode;
    
    EXPECT_EQ(opcode, static_cast<uint8_t>(OpCode::LevelComplete));
}

TEST(ProtocolAdvanced, PowerUpActivatePacket) {
    BinarySerializer serializer;
    
    serializer << static_cast<uint8_t>(OpCode::MagicByte1)
               << static_cast<uint8_t>(OpCode::MagicByte2)
               << static_cast<uint8_t>(OpCode::PowerUpActivate)
               << static_cast<uint32_t>(123) // player id
               << static_cast<uint8_t>(1);    // powerup type (PowerCannon)
    
    uint8_t magic1, magic2, opcode, powerup_type;
    uint32_t player_id;
    
    serializer >> magic1 >> magic2 >> opcode >> player_id >> powerup_type;
    
    EXPECT_EQ(opcode, static_cast<uint8_t>(OpCode::PowerUpActivate));
    EXPECT_EQ(player_id, 123u);
    EXPECT_EQ(powerup_type, 1);
}

TEST(ProtocolAdvanced, GameOverPacket) {
    BinarySerializer serializer;
    
    serializer << static_cast<uint8_t>(OpCode::MagicByte1)
               << static_cast<uint8_t>(OpCode::MagicByte2)
               << static_cast<uint8_t>(OpCode::GameOver)
               << static_cast<uint8_t>(1); // win/lose flag
    
    uint8_t magic1, magic2, opcode, result;
    serializer >> magic1 >> magic2 >> opcode >> result;
    
    EXPECT_EQ(opcode, static_cast<uint8_t>(OpCode::GameOver));
    EXPECT_EQ(result, 1);
}

TEST(ProtocolAdvanced, MultipleEntityPositions) {
    BinarySerializer serializer;
    
    // Simulate multiple entity updates in one packet
    for (uint32_t i = 0; i < 3; ++i) {
        serializer << static_cast<uint32_t>(i)
                   << static_cast<uint8_t>(EntityType::Enemy)
                   << static_cast<float>(i * 10.0)
                   << static_cast<float>(i * 20.0);
    }
    
    for (uint32_t i = 0; i < 3; ++i) {
        uint32_t id;
        uint8_t type;
        float x, y;
        
        serializer >> id >> type >> x >> y;
        
        EXPECT_EQ(id, i);
        EXPECT_EQ(type, static_cast<uint8_t>(EntityType::Enemy));
        EXPECT_FLOAT_EQ(x, static_cast<float>(i * 10.0));
        EXPECT_FLOAT_EQ(y, static_cast<float>(i * 20.0));
    }
}

TEST(ProtocolAdvanced, InvalidMagicNumber) {
    uint16_t invalid_magic = 0x1234;
    EXPECT_FALSE(MagicNumber::is_valid(invalid_magic));
    
    EXPECT_FALSE(MagicNumber::is_valid(0xFF, 0xFF));
    EXPECT_FALSE(MagicNumber::is_valid(0x00, 0x00));
}
