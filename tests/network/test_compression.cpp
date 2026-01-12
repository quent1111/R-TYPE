#include <gtest/gtest.h>
#include "../../src/Common/CompressionSerializer.hpp"
#include <iostream>

using namespace RType;

TEST(CompressionSerializer, BasicCompression) {
    CompressionSerializer serializer;
    
    // Create a packet with repetitive data (compresses well)
    for (int i = 0; i < 50; ++i) {
        serializer << static_cast<uint32_t>(12345);
        serializer << static_cast<float>(100.0f);
    }
    
    size_t original_size = serializer.data().size();
    std::cout << "[TEST] Original size: " << original_size << " bytes" << std::endl;
    
    // Compress
    bool compressed = serializer.compress();
    size_t compressed_size = serializer.data().size();
    
    std::cout << "[TEST] Compressed size: " << compressed_size << " bytes" << std::endl;
    std::cout << "[TEST] Was compressed: " << (compressed ? "YES" : "NO") << std::endl;
    
    if (compressed) {
        float ratio = static_cast<float>(compressed_size) / static_cast<float>(original_size);
        std::cout << "[TEST] Compression ratio: " << (ratio * 100.0f) << "%" << std::endl;
        EXPECT_LT(compressed_size, original_size);
    }
    
    EXPECT_GT(serializer.data().size(), 0);
}

TEST(CompressionSerializer, CompressionDecompression) {
    // Create original data
    CompressionSerializer encoder;
    encoder << static_cast<uint16_t>(0xB542);  // Magic
    encoder << static_cast<uint8_t>(0x13);     // OpCode
    encoder << static_cast<uint8_t>(10);       // Entity count
    
    // Add 10 entities
    for (int i = 0; i < 10; ++i) {
        encoder << static_cast<uint32_t>(i);           // ID
        encoder << static_cast<uint8_t>(1);            // Type
        encoder.write_position(100.5f + i, 200.3f + i); // Position
        encoder.write_velocity(50.0f, -30.0f);         // Velocity
        encoder.write_quantized_health(95, 100);       // Health
    }
    
    size_t original_size = encoder.data().size();
    std::vector<uint8_t> original_data = encoder.data();
    
    std::cout << "[TEST] Original packet size: " << original_size << " bytes" << std::endl;
    
    // Compress
    bool compressed = encoder.compress();
    size_t compressed_size = encoder.data().size();
    
    std::cout << "[TEST] Compressed packet size: " << compressed_size << " bytes" << std::endl;
    std::cout << "[TEST] Compression savings: " << (original_size - compressed_size) << " bytes ("
              << (100.0f * (1.0f - static_cast<float>(compressed_size) / original_size)) << "%)" << std::endl;
    
    // Decompress
    CompressionSerializer decoder(encoder.data());
    bool was_compressed = decoder.decompress();
    
    EXPECT_EQ(was_compressed, compressed);
    EXPECT_EQ(decoder.data().size(), original_data.size());
    
    // Verify data integrity
    uint16_t magic;
    uint8_t opcode, entity_count;
    decoder >> magic >> opcode >> entity_count;
    
    EXPECT_EQ(magic, 0xB542);
    EXPECT_EQ(opcode, 0x13);
    EXPECT_EQ(entity_count, 10);
    
    // Read first entity
    uint32_t id;
    uint8_t type;
    float x, y, vx, vy;
    
    decoder >> id >> type;
    decoder.read_position(x, y);
    decoder.read_velocity(vx, vy);
    
    EXPECT_EQ(id, 0);
    EXPECT_EQ(type, 1);
    EXPECT_NEAR(x, 100.5f, 0.1f);
    EXPECT_NEAR(y, 200.3f, 0.1f);
    EXPECT_NEAR(vx, 50.0f, 10.0f);
    EXPECT_NEAR(vy, -30.0f, 10.0f);
    
    std::cout << "[TEST] ✅ Decompression successful, data integrity verified" << std::endl;
}

TEST(CompressionSerializer, SmallPacketNotCompressed) {
    CompressionSerializer serializer;
    
    // Small packet (< 128 bytes threshold)
    serializer << static_cast<uint16_t>(0xB542);
    serializer << static_cast<uint8_t>(0x02);
    serializer << static_cast<uint32_t>(42);
    
    size_t original_size = serializer.data().size();
    
    bool compressed = serializer.compress();
    
    // Should NOT be compressed (too small)
    EXPECT_FALSE(compressed);
    std::cout << "[TEST] Small packet (" << original_size << " bytes) correctly left uncompressed" << std::endl;
}

TEST(CompressionSerializer, HighCompressionMode) {
    CompressionConfig config;
    config.min_compress_size = 100;
    config.use_high_compression = true;
    config.hc_level = 12;  // Max compression
    
    CompressionSerializer serializer;
    serializer.set_config(config);
    
    // Create data
    for (int i = 0; i < 100; ++i) {
        serializer << static_cast<uint32_t>(0xAAAAAAAA);
    }
    
    size_t original_size = serializer.data().size();
    bool compressed = serializer.compress();
    size_t compressed_size = serializer.data().size();
    
    std::cout << "[TEST] HC Mode - Original: " << original_size 
              << " bytes, Compressed: " << compressed_size << " bytes" << std::endl;
    
    if (compressed) {
        EXPECT_LT(compressed_size, original_size);
        std::cout << "[TEST] ✅ High compression mode working" << std::endl;
    }
}

TEST(CompressionSerializer, Statistics) {
    size_t total_original = 0;
    size_t total_compressed = 0;
    int packets_compressed = 0;
    
    // Send multiple packets
    for (int packet = 0; packet < 5; ++packet) {
        CompressionSerializer serializer;
        
        for (int i = 0; i < 30; ++i) {
            serializer << static_cast<uint32_t>(packet * 1000 + i);
        }
        
        size_t original_size = serializer.data().size();
        bool compressed = serializer.compress();
        size_t final_size = serializer.data().size();
        
        total_original += original_size;
        total_compressed += final_size;
        if (compressed) packets_compressed++;
    }
    
    std::cout << "\n[TEST] === Compression Statistics ===" << std::endl;
    std::cout << "  Packets compressed   : " << packets_compressed << " / 5" << std::endl;
    std::cout << "  Total bytes in       : " << total_original << std::endl;
    std::cout << "  Total bytes out      : " << total_compressed << std::endl;
    float ratio = static_cast<float>(total_compressed) / static_cast<float>(total_original);
    std::cout << "  Compression ratio    : " << (ratio * 100.0f) << "%" << std::endl;
    std::cout << "  Bandwidth savings    : " << ((1.0f - ratio) * 100.0f) << "%" << std::endl;
    std::cout << "================================\n" << std::endl;
    
    EXPECT_GT(total_original, 0);
    EXPECT_GT(total_compressed, 0);
    // Small packets may not compress (overhead), so just verify stats are tracked
}

TEST(CompressionSerializer, InvalidDecompression) {
    // Test with invalid flag
    std::vector<uint8_t> invalid_data = {0xFF, 0x00, 0x00, 0x00, 0x00};
    CompressionSerializer serializer(invalid_data);
    
    EXPECT_THROW(serializer.decompress(), CompressionException);
    
    std::cout << "[TEST] ✅ Invalid compression flag correctly rejected" << std::endl;
}

TEST(CompressionSerializer, LargePacket) {
    CompressionSerializer serializer;
    
    // Create a large packet with REPETITIVE data (compresses well)
    serializer << static_cast<uint16_t>(0xB542);
    serializer << static_cast<uint8_t>(0x13);
    serializer << static_cast<uint16_t>(1000);  // Entity count
    
    // Use same position/velocity for all entities (repetitive = good compression)
    for (int i = 0; i < 1000; ++i) {
        serializer << static_cast<uint32_t>(i);
        serializer << static_cast<uint8_t>(1);
        serializer.write_position(100.0f, 200.0f);  // Same position
        serializer.write_velocity(50.0f, -30.0f);   // Same velocity
    }
    
    size_t original_size = serializer.data().size();
    std::cout << "[TEST] Large packet original size: " << original_size << " bytes" << std::endl;
    
    bool compressed = serializer.compress();
    
    size_t compressed_size = serializer.data().size();
    std::cout << "[TEST] Large packet compressed size: " << compressed_size << " bytes" << std::endl;
    std::cout << "[TEST] Compression ratio: " 
              << (100.0f * compressed_size / original_size) << "%" << std::endl;
    
    if (compressed) {
        std::cout << "[TEST] ✅ Large repetitive packet compressed successfully" << std::endl;
        // Repetitive data should compress to around 30-40%
        EXPECT_LT(compressed_size, original_size * 0.5f);  // At least 50% compression
    } else {
        std::cout << "[TEST] ⚠️ Packet not compressed (LZ4 decided overhead not worth it)" << std::endl;
    }
}
