#include <gtest/gtest.h>
#include "../../src/Common/BinarySerializer.hpp"

using namespace RType;

TEST(SerializerEdgeCases, ZeroValues) {
    BinarySerializer s;
    s << static_cast<uint8_t>(0) << static_cast<uint16_t>(0) 
      << static_cast<uint32_t>(0) << static_cast<float>(0.0f);
    
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    float f;
    
    s >> u8 >> u16 >> u32 >> f;
    
    EXPECT_EQ(u8, 0);
    EXPECT_EQ(u16, 0);
    EXPECT_EQ(u32, 0);
    EXPECT_FLOAT_EQ(f, 0.0f);
}

TEST(SerializerEdgeCases, MaxValues) {
    BinarySerializer s;
    s << static_cast<uint8_t>(255) << static_cast<uint16_t>(65535) 
      << static_cast<uint32_t>(4294967295);
    
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    
    s >> u8 >> u16 >> u32;
    
    EXPECT_EQ(u8, 255);
    EXPECT_EQ(u16, 65535);
    EXPECT_EQ(u32, 4294967295);
}

TEST(SerializerEdgeCases, MinNegativeValues) {
    BinarySerializer s;
    s << static_cast<int8_t>(-128) << static_cast<int16_t>(-32768) 
      << static_cast<int32_t>(-2147483648);
    
    int8_t i8;
    int16_t i16;
    int32_t i32;
    
    s >> i8 >> i16 >> i32;
    
    EXPECT_EQ(i8, -128);
    EXPECT_EQ(i16, -32768);
    EXPECT_EQ(i32, -2147483648);
}

TEST(SerializerEdgeCases, VerySmallFloat) {
    BinarySerializer s;
    float small = 0.000001f;
    s << small;
    
    float result;
    s >> result;
    
    EXPECT_FLOAT_EQ(result, small);
}

TEST(SerializerEdgeCases, VeryLargeFloat) {
    BinarySerializer s;
    float large = 1000000.0f;
    s << large;
    
    float result;
    s >> result;
    
    EXPECT_FLOAT_EQ(result, large);
}

TEST(SerializerEdgeCases, NegativeFloat) {
    BinarySerializer s;
    float neg = -999.999f;
    s << neg;
    
    float result;
    s >> result;
    
    EXPECT_FLOAT_EQ(result, neg);
}

TEST(SerializerEdgeCases, StringWithSpecialChars) {
    BinarySerializer s;
    std::string special = "Hello\nWorld\t!@#$%";
    s << special;
    
    std::string result;
    s >> result;
    
    EXPECT_EQ(result, special);
}

TEST(SerializerEdgeCases, UnicodeString) {
    BinarySerializer s;
    std::string unicode = "こんにちは世界"; // Japanese
    s << unicode;
    
    std::string result;
    s >> result;
    
    EXPECT_EQ(result, unicode);
}

TEST(SerializerEdgeCases, SingleChar) {
    BinarySerializer s;
    char c = 'A';
    s << c;
    
    char result;
    s >> result;
    
    EXPECT_EQ(result, c);
}

TEST(SerializerEdgeCases, Bool) {
    BinarySerializer s;
    bool b1 = true;
    bool b2 = false;
    s << b1 << b2;
    
    bool r1, r2;
    s >> r1 >> r2;
    
    EXPECT_TRUE(r1);
    EXPECT_FALSE(r2);
}

TEST(SerializerEdgeCases, Int64) {
    BinarySerializer s;
    int64_t big = 9223372036854775807LL;
    s << big;
    
    int64_t result;
    s >> result;
    
    EXPECT_EQ(result, big);
}

TEST(SerializerEdgeCases, UInt64) {
    BinarySerializer s;
    uint64_t big = 18446744073709551615ULL;
    s << big;
    
    uint64_t result;
    s >> result;
    
    EXPECT_EQ(result, big);
}

TEST(SerializerEdgeCases, SequentialReads) {
    BinarySerializer s;
    for (int i = 0; i < 10; ++i) {
        s << static_cast<uint32_t>(i);
    }
    
    for (int i = 0; i < 10; ++i) {
        uint32_t value;
        s >> value;
        EXPECT_EQ(value, i);
    }
}

TEST(SerializerEdgeCases, InterleavedTypes) {
    BinarySerializer s;
    s << static_cast<uint8_t>(1) 
      << static_cast<float>(2.0f) 
      << static_cast<uint16_t>(3) 
      << static_cast<double>(4.0) 
      << static_cast<uint32_t>(5);
    
    uint8_t u8;
    float f;
    uint16_t u16;
    double d;
    uint32_t u32;
    
    s >> u8 >> f >> u16 >> d >> u32;
    
    EXPECT_EQ(u8, 1);
    EXPECT_FLOAT_EQ(f, 2.0f);
    EXPECT_EQ(u16, 3);
    EXPECT_DOUBLE_EQ(d, 4.0);
    EXPECT_EQ(u32, 5);
}

TEST(SerializerEdgeCases, PartialRead) {
    BinarySerializer s;
    s << static_cast<uint32_t>(1) 
      << static_cast<uint32_t>(2) 
      << static_cast<uint32_t>(3);
    
    uint32_t v1, v2;
    s >> v1 >> v2;
    
    EXPECT_EQ(v1, 1);
    EXPECT_EQ(v2, 2);
    EXPECT_EQ(s.remaining(), sizeof(uint32_t));
}

TEST(SerializerEdgeCases, RemainingAfterReset) {
    BinarySerializer s;
    s << static_cast<uint32_t>(42);
    
    uint32_t value;
    s >> value;
    
    EXPECT_EQ(s.remaining(), 0);
    
    s.reset_read_position();
    EXPECT_EQ(s.remaining(), sizeof(uint32_t));
}
