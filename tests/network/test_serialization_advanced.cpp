#include <gtest/gtest.h>
#include "../../src/Common/BinarySerializer.hpp"

using namespace RType;

TEST(BinarySerializerAdvanced, MultipleStrings) {
    BinarySerializer serializer;
    
    std::string s1 = "Player1";
    std::string s2 = "Player2";
    std::string s3 = "Player3";
    
    serializer << s1 << s2 << s3;
    
    std::string r1, r2, r3;
    serializer >> r1 >> r2 >> r3;
    
    EXPECT_EQ(r1, s1);
    EXPECT_EQ(r2, s2);
    EXPECT_EQ(r3, s3);
}

TEST(BinarySerializerAdvanced, EmptyString) {
    BinarySerializer serializer;
    std::string empty = "";
    
    serializer << empty;
    
    std::string result;
    serializer >> result;
    
    EXPECT_EQ(result, "");
    EXPECT_TRUE(result.empty());
}

TEST(BinarySerializerAdvanced, LargeString) {
    BinarySerializer serializer;
    std::string large(1000, 'A');
    
    serializer << large;
    
    std::string result;
    serializer >> result;
    
    EXPECT_EQ(result.size(), 1000);
    EXPECT_EQ(result, large);
}

TEST(BinarySerializerAdvanced, MixedTypes) {
    BinarySerializer serializer;
    
    uint8_t u8 = 255;
    int32_t i32 = -12345;
    float f = -3.14159f;
    std::string str = "Mixed";
    uint16_t u16 = 9999;
    
    serializer << u8 << i32 << f << str << u16;
    
    uint8_t r_u8;
    int32_t r_i32;
    float r_f;
    std::string r_str;
    uint16_t r_u16;
    
    serializer >> r_u8 >> r_i32 >> r_f >> r_str >> r_u16;
    
    EXPECT_EQ(r_u8, u8);
    EXPECT_EQ(r_i32, i32);
    EXPECT_FLOAT_EQ(r_f, f);
    EXPECT_EQ(r_str, str);
    EXPECT_EQ(r_u16, u16);
}

TEST(BinarySerializerAdvanced, CanRead) {
    BinarySerializer serializer;
    serializer << static_cast<uint32_t>(42);
    
    EXPECT_TRUE(serializer.can_read(sizeof(uint32_t)));
    EXPECT_FALSE(serializer.can_read(sizeof(uint32_t) + 1));
    
    uint32_t value;
    serializer >> value;
    
    EXPECT_FALSE(serializer.can_read(1));
}

TEST(BinarySerializerAdvanced, ClearAndReuse) {
    BinarySerializer serializer;
    
    serializer << static_cast<uint8_t>(1);
    EXPECT_EQ(serializer.size(), 1);
    
    serializer.clear();
    EXPECT_EQ(serializer.size(), 0);
    EXPECT_EQ(serializer.read_position(), 0);
    
    serializer << static_cast<uint16_t>(2);
    EXPECT_EQ(serializer.size(), 2);
}

TEST(BinarySerializerAdvanced, Reserve) {
    BinarySerializer serializer;
    serializer.reserve(1024);
    
    for (int i = 0; i < 100; ++i) {
        serializer << static_cast<uint32_t>(i);
    }
    
    EXPECT_EQ(serializer.size(), 100 * sizeof(uint32_t));
}

TEST(BinarySerializerAdvanced, WriteReadBytes) {
    BinarySerializer serializer;
    
    uint8_t data[5] = {1, 2, 3, 4, 5};
    serializer.write_bytes(data, 5);
    
    uint8_t result[5] = {0};
    serializer.read_bytes(result, 5);
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(result[i], data[i]);
    }
}

TEST(BinarySerializerAdvanced, ConstructFromVector) {
    std::vector<uint8_t> data = {0x42, 0xB5, 0x01, 0x10, 0x20, 0x30};
    BinarySerializer serializer(data);
    
    EXPECT_EQ(serializer.size(), 6);
    
    uint8_t byte1, byte2, byte3;
    serializer >> byte1 >> byte2 >> byte3;
    
    EXPECT_EQ(byte1, 0x42);
    EXPECT_EQ(byte2, 0xB5);
    EXPECT_EQ(byte3, 0x01);
}

TEST(BinarySerializerAdvanced, NegativeNumbers) {
    BinarySerializer serializer;
    
    int8_t i8 = -128;
    int16_t i16 = -32000;
    int32_t i32 = -2000000000;
    
    serializer << i8 << i16 << i32;
    
    int8_t r_i8;
    int16_t r_i16;
    int32_t r_i32;
    
    serializer >> r_i8 >> r_i16 >> r_i32;
    
    EXPECT_EQ(r_i8, i8);
    EXPECT_EQ(r_i16, i16);
    EXPECT_EQ(r_i32, i32);
}

TEST(BinarySerializerAdvanced, DoubleValues) {
    BinarySerializer serializer;
    
    double d1 = 3.141592653589793;
    double d2 = -2.718281828459045;
    
    serializer << d1 << d2;
    
    double r_d1, r_d2;
    serializer >> r_d1 >> r_d2;
    
    EXPECT_DOUBLE_EQ(r_d1, d1);
    EXPECT_DOUBLE_EQ(r_d2, d2);
}
