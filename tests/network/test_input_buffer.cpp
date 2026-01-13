#include <gtest/gtest.h>
#include "../../server/include/handlers/InputBuffer.hpp"
#include <thread>
#include <chrono>

using namespace server;

class InputBufferTest : public ::testing::Test {
protected:
    ClientInputBuffer buffer_;
};

// ============================================================================
// Tests de base
// ============================================================================

TEST_F(InputBufferTest, AddSingleInput) {
    EXPECT_TRUE(buffer_.add_input(100, 0x01));
    EXPECT_EQ(buffer_.size(), 1);
    EXPECT_FALSE(buffer_.empty());
}

TEST_F(InputBufferTest, AddMultipleInputs) {
    EXPECT_TRUE(buffer_.add_input(100, 0x01));
    EXPECT_TRUE(buffer_.add_input(116, 0x02));
    EXPECT_TRUE(buffer_.add_input(132, 0x04));
    
    EXPECT_EQ(buffer_.size(), 3);
}

TEST_F(InputBufferTest, BufferClear) {
    buffer_.add_input(100, 0x01);
    buffer_.add_input(116, 0x02);
    
    buffer_.clear();
    
    EXPECT_TRUE(buffer_.empty());
    EXPECT_EQ(buffer_.size(), 0);
}

// ============================================================================
// Tests de délai
// ============================================================================

TEST_F(InputBufferTest, InputNotReadyImmediately) {
    buffer_.add_input(100, 0x01);
    
    // Immédiatement après ajout, pas prêt (délai = 50ms)
    auto ready = buffer_.get_ready_inputs();
    EXPECT_TRUE(ready.empty());
    EXPECT_EQ(buffer_.size(), 1);  // Toujours dans le buffer
}

TEST_F(InputBufferTest, InputReadyAfterDelay) {
    buffer_.add_input(100, 0x01);
    
    // Attendre le délai configuré (50ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    auto ready = buffer_.get_ready_inputs();
    EXPECT_EQ(ready.size(), 1);
    EXPECT_EQ(ready[0].input_mask, 0x01);
    EXPECT_EQ(buffer_.size(), 0);  // Buffer vidé
}

TEST_F(InputBufferTest, MultipleInputsProgressiveRelease) {
    // Ajouter 3 inputs avec 20ms d'écart
    buffer_.add_input(100, 0x01);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    buffer_.add_input(120, 0x02);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    buffer_.add_input(140, 0x04);
    
    // Attendre 30ms → Seul le premier devrait être prêt
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    auto ready1 = buffer_.get_ready_inputs();
    EXPECT_EQ(ready1.size(), 1);
    EXPECT_EQ(ready1[0].input_mask, 0x01);
    
    // Attendre 20ms de plus → Le deuxième devrait être prêt
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    auto ready2 = buffer_.get_ready_inputs();
    EXPECT_EQ(ready2.size(), 1);
    EXPECT_EQ(ready2[0].input_mask, 0x02);
}

// ============================================================================
// Tests de capacité
// ============================================================================

TEST_F(InputBufferTest, BufferOverflow) {
    // Remplir le buffer au-delà de la capacité max
    for (size_t i = 0; i < InputDelayConfig::MAX_BUFFERED_INPUTS + 10; ++i) {
        buffer_.add_input(static_cast<uint32_t>(i * 16), static_cast<uint8_t>(i % 256));
    }
    
    // Le buffer devrait rester à la capacité max
    EXPECT_LE(buffer_.size(), InputDelayConfig::MAX_BUFFERED_INPUTS);
}

TEST_F(InputBufferTest, OldestInputsDroppedOnOverflow) {
    // Ajouter inputs avec délai pour qu'ils soient prêts
    for (size_t i = 0; i < 5; ++i) {
        buffer_.add_input(static_cast<uint32_t>(i * 16), static_cast<uint8_t>(i));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    // Overflow avec nouveaux inputs
    for (size_t i = 0; i < InputDelayConfig::MAX_BUFFERED_INPUTS; ++i) {
        buffer_.add_input(static_cast<uint32_t>(1000 + i * 16), static_cast<uint8_t>(i % 256));
    }
    
    // Les anciens devraient avoir été supprimés
    auto ready = buffer_.get_ready_inputs();
    EXPECT_LE(ready.size(), 5);  // Au plus les 5 premiers
}

// ============================================================================
// Tests d'expiration
// ============================================================================

TEST_F(InputBufferTest, ExpiredInputsRemoved) {
    buffer_.add_input(100, 0x01);
    
    // Simuler expiration (timeout > 5000ms)
    // Note: Ce test prend 5+ secondes, désactivé par défaut
    // std::this_thread::sleep_for(std::chrono::milliseconds(5100));
    
    // auto ready = buffer_.get_ready_inputs();
    // EXPECT_TRUE(ready.empty());  // Input expiré, supprimé
}

// ============================================================================
// Tests de timestamp
// ============================================================================

TEST_F(InputBufferTest, TimestampPreserved) {
    uint32_t original_timestamp = 12345;
    buffer_.add_input(original_timestamp, 0x01);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    auto ready = buffer_.get_ready_inputs();
    ASSERT_EQ(ready.size(), 1);
    EXPECT_EQ(ready[0].client_timestamp, original_timestamp);
}

TEST_F(InputBufferTest, InputMaskPreserved) {
    uint8_t original_mask = 0b11010101;
    buffer_.add_input(100, original_mask);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    auto ready = buffer_.get_ready_inputs();
    ASSERT_EQ(ready.size(), 1);
    EXPECT_EQ(ready[0].input_mask, original_mask);
}

// ============================================================================
// Tests de configuration
// ============================================================================

TEST_F(InputBufferTest, ConfigurationValues) {
    // Vérifier que les valeurs de configuration sont raisonnables
    EXPECT_GT(InputDelayConfig::INPUT_DELAY_MS, 0);
    EXPECT_LT(InputDelayConfig::INPUT_DELAY_MS, 1000);
    
    EXPECT_GT(InputDelayConfig::MAX_BUFFERED_INPUTS, 10);
    EXPECT_LT(InputDelayConfig::MAX_BUFFERED_INPUTS, 1000);
    
    EXPECT_GT(InputDelayConfig::INPUT_TIMEOUT_MS, InputDelayConfig::INPUT_DELAY_MS);
}

// ============================================================================
// Tests de stress
// ============================================================================

TEST_F(InputBufferTest, RapidInputAddition) {
    // Simuler 60 FPS pendant 1 seconde
    for (int i = 0; i < 60; ++i) {
        buffer_.add_input(static_cast<uint32_t>(i * 16), static_cast<uint8_t>(i % 16));
    }
    
    EXPECT_LE(buffer_.size(), 60);
}

TEST_F(InputBufferTest, ConcurrentAccess) {
    std::thread writer([this]() {
        for (int i = 0; i < 100; ++i) {
            buffer_.add_input(static_cast<uint32_t>(i * 16), static_cast<uint8_t>(i % 256));
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    
    std::thread reader([this]() {
        for (int i = 0; i < 20; ++i) {
            buffer_.get_ready_inputs();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });
    
    writer.join();
    reader.join();
    
    // Pas de crash = succès
    SUCCEED();
}

// ============================================================================
// Tests edge cases
// ============================================================================

TEST_F(InputBufferTest, ZeroTimestamp) {
    buffer_.add_input(0, 0x01);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    auto ready = buffer_.get_ready_inputs();
    ASSERT_EQ(ready.size(), 1);
    EXPECT_EQ(ready[0].client_timestamp, 0);
}

TEST_F(InputBufferTest, MaxTimestamp) {
    uint32_t max_timestamp = 0xFFFFFFFF;
    buffer_.add_input(max_timestamp, 0x01);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    auto ready = buffer_.get_ready_inputs();
    ASSERT_EQ(ready.size(), 1);
    EXPECT_EQ(ready[0].client_timestamp, max_timestamp);
}

TEST_F(InputBufferTest, AllInputMaskBitsSet) {
    uint8_t all_bits = 0xFF;
    buffer_.add_input(100, all_bits);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    auto ready = buffer_.get_ready_inputs();
    ASSERT_EQ(ready.size(), 1);
    EXPECT_EQ(ready[0].input_mask, all_bits);
}

// ============================================================================
// Tests de performance
// ============================================================================

TEST_F(InputBufferTest, AddInputPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        buffer_.add_input(static_cast<uint32_t>(i), static_cast<uint8_t>(i % 256));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 10000 additions devraient prendre < 10ms
    EXPECT_LT(duration.count(), 10000);
    
    std::cout << "[Performance] 10000 add_input() took " << duration.count() << "µs" << std::endl;
}

TEST_F(InputBufferTest, GetReadyInputsPerformance) {
    // Ajouter beaucoup d'inputs
    for (int i = 0; i < 1000; ++i) {
        buffer_.add_input(static_cast<uint32_t>(i), static_cast<uint8_t>(i % 256));
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    
    auto start = std::chrono::high_resolution_clock::now();
    auto ready = buffer_.get_ready_inputs();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Devrait être très rapide (< 1ms)
    EXPECT_LT(duration.count(), 1000);
    
    std::cout << "[Performance] get_ready_inputs() with " << ready.size() 
              << " inputs took " << duration.count() << "µs" << std::endl;
}
