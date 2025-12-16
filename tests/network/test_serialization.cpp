/**
 * @file test_serialization.cpp
 * @brief Unit tests for data serialization
 */

#include <gtest/gtest.h>
#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"

using namespace RType;

TEST(BinarySerializer, WriteAndReadPrimitives) {
    BinarySerializer serializer;

    uint8_t u8 = 42;
    uint16_t u16 = 1234;
    uint32_t u32 = 567890;
    float f = 3.14159f;

    serializer << u8 << u16 << u32 << f;

    uint8_t r_u8;
    uint16_t r_u16;
    uint32_t r_u32;
    float r_f;

    serializer >> r_u8 >> r_u16 >> r_u32 >> r_f;

    EXPECT_EQ(r_u8, u8);
    EXPECT_EQ(r_u16, u16);
    EXPECT_EQ(r_u32, u32);
    EXPECT_FLOAT_EQ(r_f, f);
}

TEST(BinarySerializer, WriteAndReadString) {
    BinarySerializer serializer;
    std::string original = "Hello R-TYPE!";

    serializer << original;

    std::string result;
    serializer >> result;

    EXPECT_EQ(result, original);
}

TEST(BinarySerializer, BufferUnderflowThrows) {
    BinarySerializer serializer;
    serializer << static_cast<uint8_t>(1);

    uint32_t value;
    EXPECT_THROW(serializer >> value, SerializationException);
}

TEST(BinarySerializer, ResetReadPosition) {
    BinarySerializer serializer;
    serializer << static_cast<uint8_t>(42);

    uint8_t value1;
    serializer >> value1;
    EXPECT_EQ(value1, 42);

    serializer.reset_read_position();
    uint8_t value2;
    serializer >> value2;
    EXPECT_EQ(value2, 42);
}

TEST(BinarySerializer, RemainingBytes) {
    BinarySerializer serializer;
    serializer << static_cast<uint32_t>(100);
    serializer << static_cast<uint32_t>(200);

    EXPECT_EQ(serializer.remaining(), 8);

    uint32_t val;
    serializer >> val;
    EXPECT_EQ(serializer.remaining(), 4);

    serializer >> val;
    EXPECT_EQ(serializer.remaining(), 0);
}

TEST(BinarySerializer, SerializeEntityPosition) {
    BinarySerializer serializer;

    uint8_t magic1 = static_cast<uint8_t>(OpCode::MagicByte1);
    uint8_t magic2 = static_cast<uint8_t>(OpCode::MagicByte2);
    uint8_t opcode = static_cast<uint8_t>(OpCode::EntityPosition);
    uint32_t entity_id = 123;
    uint8_t entity_type = static_cast<uint8_t>(EntityType::Player);
    float x = 100.5f;
    float y = 200.3f;
    float vx = 10.0f;
    float vy = -5.0f;

    serializer << magic1 << magic2 << opcode 
               << entity_id << entity_type 
               << x << y << vx << vy;

    uint8_t r_magic1, r_magic2, r_opcode, r_type;
    uint32_t r_id;
    float r_x, r_y, r_vx, r_vy;

    serializer >> r_magic1 >> r_magic2 >> r_opcode 
               >> r_id >> r_type 
               >> r_x >> r_y >> r_vx >> r_vy;

    EXPECT_EQ(r_magic1, magic1);
    EXPECT_EQ(r_magic2, magic2);
    EXPECT_EQ(r_opcode, opcode);
    EXPECT_EQ(r_id, entity_id);
    EXPECT_EQ(r_type, entity_type);
    EXPECT_FLOAT_EQ(r_x, x);
    EXPECT_FLOAT_EQ(r_y, y);
    EXPECT_FLOAT_EQ(r_vx, vx);
    EXPECT_FLOAT_EQ(r_vy, vy);
}

