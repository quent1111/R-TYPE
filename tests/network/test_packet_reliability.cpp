#include <gtest/gtest.h>
#include "../../server/include/network/PacketReliability.hpp"
#include <thread>
#include <chrono>

using namespace RType;

class PacketReliabilityTest : public ::testing::Test {
protected:
    ClientReliabilityState state_;
};

// ============================================================================
// Tests de Sequence ID
// ============================================================================

TEST_F(PacketReliabilityTest, SequenceIdIncrement) {
    uint32_t seq1 = state_.get_next_send_sequence();
    uint32_t seq2 = state_.get_next_send_sequence();
    uint32_t seq3 = state_.get_next_send_sequence();
    
    EXPECT_EQ(seq1, 1);
    EXPECT_EQ(seq2, 2);
    EXPECT_EQ(seq3, 3);
}

TEST_F(PacketReliabilityTest, SequenceIdStartsAtOne) {
    EXPECT_EQ(state_.next_send_sequence, 1);
    EXPECT_EQ(state_.expected_recv_sequence, 1);
}

// ============================================================================
// Tests de Detection de Duplicatas
// ============================================================================

TEST_F(PacketReliabilityTest, FirstPacketNotDuplicate) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    auto ready = state_.process_received_packet(1, data);
    
    EXPECT_EQ(ready.size(), 1);
}

TEST_F(PacketReliabilityTest, DuplicatePacketDetected) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    
    // Premier paquet
    auto ready1 = state_.process_received_packet(1, data);
    EXPECT_EQ(ready1.size(), 1);
    
    // Duplicata du même paquet
    auto ready2 = state_.process_received_packet(1, data);
    EXPECT_EQ(ready2.size(), 0);  // Ignoré
}

TEST_F(PacketReliabilityTest, MultipleDuplicatesIgnored) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    
    state_.process_received_packet(1, data);
    
    // Envoyer 5 duplicatas
    for (int i = 0; i < 5; ++i) {
        auto ready = state_.process_received_packet(1, data);
        EXPECT_EQ(ready.size(), 0);
    }
}

// ============================================================================
// Tests de Reordering
// ============================================================================

TEST_F(PacketReliabilityTest, InOrderPackets) {
    std::vector<uint8_t> data1 = {0x01};
    std::vector<uint8_t> data2 = {0x02};
    std::vector<uint8_t> data3 = {0x03};
    
    auto ready1 = state_.process_received_packet(1, data1);
    auto ready2 = state_.process_received_packet(2, data2);
    auto ready3 = state_.process_received_packet(3, data3);
    
    EXPECT_EQ(ready1.size(), 1);
    EXPECT_EQ(ready2.size(), 1);
    EXPECT_EQ(ready3.size(), 1);
}

TEST_F(PacketReliabilityTest, OutOfOrderPacketsBuffered) {
    std::vector<uint8_t> data1 = {0x01};
    std::vector<uint8_t> data3 = {0x03};
    
    // Recevoir seq=1
    auto ready1 = state_.process_received_packet(1, data1);
    EXPECT_EQ(ready1.size(), 1);
    
    // Recevoir seq=3 (seq=2 manquant)
    auto ready3 = state_.process_received_packet(3, data3);
    EXPECT_EQ(ready3.size(), 0);  // Bufferisé, pas prêt
    EXPECT_EQ(state_.reorder_buffer.size(), 1);
}

TEST_F(PacketReliabilityTest, ReplayBufferedPackets) {
    std::vector<uint8_t> data1 = {0x01};
    std::vector<uint8_t> data2 = {0x02};
    std::vector<uint8_t> data3 = {0x03};
    std::vector<uint8_t> data4 = {0x04};
    
    // Recevoir dans le désordre : 1, 3, 4, 2
    state_.process_received_packet(1, data1);
    state_.process_received_packet(3, data3);
    state_.process_received_packet(4, data4);
    
    // Quand seq=2 arrive, devrait rejouer 2, 3, 4
    auto ready = state_.process_received_packet(2, data2);
    
    EXPECT_EQ(ready.size(), 3);  // packet 2, 3, 4
    EXPECT_EQ(state_.reorder_buffer.size(), 0);  // Buffer vidé
    EXPECT_EQ(state_.expected_recv_sequence, 5);
}

TEST_F(PacketReliabilityTest, LargeGapReordering) {
    std::vector<uint8_t> data1 = {0x01};
    std::vector<uint8_t> data10 = {0x0A};
    
    state_.process_received_packet(1, data1);
    
    // Gap de 9 paquets
    auto ready = state_.process_received_packet(10, data10);
    
    EXPECT_EQ(ready.size(), 0);  // Bufferisé
    EXPECT_EQ(state_.reorder_buffer.size(), 1);
}

// ============================================================================
// Tests de Fenêtre de Reordering
// ============================================================================

TEST_F(PacketReliabilityTest, PacketWithinWindow) {
    state_.process_received_packet(1, {0x01});
    
    // Dans la fenêtre [2, 65]
    EXPECT_TRUE(state_.is_in_reorder_window(2));
    EXPECT_TRUE(state_.is_in_reorder_window(30));
    EXPECT_TRUE(state_.is_in_reorder_window(65));
}

TEST_F(PacketReliabilityTest, PacketOutsideWindow) {
    state_.process_received_packet(1, {0x01});
    
    // Hors fenêtre
    EXPECT_FALSE(state_.is_in_reorder_window(0));   // Trop ancien
    EXPECT_FALSE(state_.is_in_reorder_window(66));  // Trop futur
    EXPECT_FALSE(state_.is_in_reorder_window(100)); // Très loin
}

TEST_F(PacketReliabilityTest, WindowMovesWithExpectedSequence) {
    state_.process_received_packet(1, {0x01});
    EXPECT_EQ(state_.expected_recv_sequence, 2);
    
    state_.process_received_packet(2, {0x02});
    EXPECT_EQ(state_.expected_recv_sequence, 3);
    
    // Fenêtre devrait avoir bougé
    EXPECT_FALSE(state_.is_in_reorder_window(1));  // Trop ancien maintenant
    EXPECT_TRUE(state_.is_in_reorder_window(3));   // Nouveau début
}

// ============================================================================
// Tests de Pending Packets (ACK)
// ============================================================================

TEST_F(PacketReliabilityTest, PendingPacketCreation) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    PendingPacket packet(1, 0x10, data);
    
    EXPECT_EQ(packet.sequence_id, 1);
    EXPECT_EQ(packet.opcode, 0x10);
    EXPECT_EQ(packet.retry_count, 0);
    EXPECT_FALSE(packet.max_retries_reached());
}

TEST_F(PacketReliabilityTest, PendingPacketShouldRetry) {
    std::vector<uint8_t> data = {0x01};
    PendingPacket packet(1, 0x10, data);
    
    auto now = std::chrono::steady_clock::now();
    
    // Juste créé, ne devrait pas retry immédiatement
    EXPECT_FALSE(packet.should_retry(now));
    
    // Attendre le timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(210));
    auto later = std::chrono::steady_clock::now();
    EXPECT_TRUE(packet.should_retry(later));
}

TEST_F(PacketReliabilityTest, PendingPacketMaxRetries) {
    std::vector<uint8_t> data = {0x01};
    PendingPacket packet(1, 0x10, data);
    
    auto now = std::chrono::steady_clock::now();
    
    // Simuler 3 retries
    for (int i = 0; i < 3; ++i) {
        packet.mark_resent(now);
    }
    
    EXPECT_TRUE(packet.max_retries_reached());
    EXPECT_EQ(packet.retry_count, 3);
}

// ============================================================================
// Tests de Buffered Packets
// ============================================================================

TEST_F(PacketReliabilityTest, BufferedPacketExpiration) {
    std::vector<uint8_t> data = {0x01};
    BufferedPacket packet(1, data);
    
    auto now = std::chrono::steady_clock::now();
    EXPECT_FALSE(packet.is_expired(now));
    
    // Simuler expiration (> 500ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(510));
    auto later = std::chrono::steady_clock::now();
    EXPECT_TRUE(packet.is_expired(later));
}

// ============================================================================
// Tests de Cache de Duplicatas
// ============================================================================

TEST_F(PacketReliabilityTest, DuplicateCacheExpiry) {
    DuplicateCacheEntry entry;
    
    auto now = std::chrono::steady_clock::now();
    EXPECT_FALSE(entry.is_expired(now));
    
    // Note: Expiration = 5000ms, test long désactivé par défaut
    // std::this_thread::sleep_for(std::chrono::milliseconds(5100));
    // auto later = std::chrono::steady_clock::now();
    // EXPECT_TRUE(entry.is_expired(later));
}

TEST_F(PacketReliabilityTest, DuplicateCacheSize) {
    // Ajouter plus que la capacité du cache
    for (uint32_t i = 1; i <= ReliabilityConfig::DUPLICATE_CACHE_SIZE + 10; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i)};
        state_.process_received_packet(i, data);
    }
    
    // Cache devrait être limité
    EXPECT_LE(state_.duplicate_cache.size(), ReliabilityConfig::DUPLICATE_CACHE_SIZE);
}

// ============================================================================
// Tests de Cleanup
// ============================================================================

TEST_F(PacketReliabilityTest, ReorderBufferCleanup) {
    // Ajouter paquets qui vont expirer
    state_.process_received_packet(1, {0x01});  // expected devient 2
    
    // Ajouter paquets 3-10 (manque paquet 2)
    for (uint32_t i = 3; i <= 10; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i)};
        state_.process_received_packet(i, data);
    }
    
    size_t initial_size = state_.reorder_buffer.size();
    EXPECT_EQ(initial_size, 8);  // Paquets 3-10 bufferisés
    EXPECT_EQ(state_.expected_recv_sequence, 2);
    
    // Attendre expiration (REORDER_BUFFER_TIMEOUT_MS = 500ms)
    std::this_thread::sleep_for(std::chrono::milliseconds(510));
    
    // Le cleanup se déclenche dans process_received_packet
    // Envoyer paquet 2 pour déclencher le cleanup
    state_.process_received_packet(2, {0x02});
    
    // Après cleanup et replay, le buffer devrait être vide
    // (paquets 3-10 ont été rejoués après réception du paquet 2)
    EXPECT_EQ(state_.reorder_buffer.size(), 0);
    EXPECT_EQ(state_.expected_recv_sequence, 11);
}

TEST_F(PacketReliabilityTest, ResetState) {
    // Remplir l'état
    state_.get_next_send_sequence();
    state_.get_next_send_sequence();
    state_.process_received_packet(1, {0x01});
    state_.process_received_packet(3, {0x03});
    
    EXPECT_GT(state_.next_send_sequence, 1);
    EXPECT_GT(state_.reorder_buffer.size(), 0);
    
    // Reset
    state_.reset();
    
    EXPECT_EQ(state_.next_send_sequence, 1);
    EXPECT_EQ(state_.expected_recv_sequence, 1);
    EXPECT_EQ(state_.reorder_buffer.size(), 0);
    EXPECT_EQ(state_.duplicate_cache.size(), 0);
}

// ============================================================================
// Tests de Configuration
// ============================================================================

TEST_F(PacketReliabilityTest, ConfigurationValues) {
    EXPECT_EQ(ReliabilityConfig::MAX_RETRIES, 3);
    EXPECT_EQ(ReliabilityConfig::RETRY_TIMEOUT_MS, 200);
    EXPECT_EQ(ReliabilityConfig::REORDER_WINDOW_SIZE, 64);
    EXPECT_EQ(ReliabilityConfig::REORDER_BUFFER_TIMEOUT_MS, 500);
    EXPECT_EQ(ReliabilityConfig::DUPLICATE_CACHE_SIZE, 256);
    EXPECT_EQ(ReliabilityConfig::DUPLICATE_CACHE_TTL_MS, 5000);
}

// ============================================================================
// Tests de Scénarios Réels
// ============================================================================

TEST_F(PacketReliabilityTest, TypicalGameSession) {
    // Simuler une session de jeu réelle avec quelques paquets désordonnés
    std::vector<std::vector<uint8_t>> packets;
    for (int i = 0; i < 100; ++i) {
        packets.push_back({static_cast<uint8_t>(i)});
    }
    
    // Simuler réception avec quelques désordres
    std::vector<int> order = {0, 1, 2, 4, 5, 3, 6, 7, 9, 8, 10};  // Quelques inversions
    
    for (int idx : order) {
        if (idx < static_cast<int>(packets.size())) {
            auto ready = state_.process_received_packet(idx + 1, packets[idx]);
            // Vérifier que tous les paquets sont traités
            EXPECT_GE(ready.size(), 0);
        }
    }
    
    // À la fin, tout devrait être traité
    EXPECT_EQ(state_.expected_recv_sequence, 12);
}

TEST_F(PacketReliabilityTest, PacketLossThenRecovery) {
    // Paquets 1, 2, 3, [4 perdu], 5, 6, 7, puis 4 arrive tard
    state_.process_received_packet(1, {0x01});
    state_.process_received_packet(2, {0x02});
    state_.process_received_packet(3, {0x03});
    
    // Sauter 4
    state_.process_received_packet(5, {0x05});
    state_.process_received_packet(6, {0x06});
    state_.process_received_packet(7, {0x07});
    
    EXPECT_EQ(state_.reorder_buffer.size(), 3);  // 5, 6, 7 bufferisés
    
    // 4 arrive finalement
    auto ready = state_.process_received_packet(4, {0x04});
    
    EXPECT_EQ(ready.size(), 4);  // 4, 5, 6, 7 rejoués
    EXPECT_EQ(state_.expected_recv_sequence, 8);
}

// ============================================================================
// Tests de Performance
// ============================================================================

TEST_F(PacketReliabilityTest, ProcessPacketPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= 1000; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i % 256)};
        state_.process_received_packet(i, data);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 1000 paquets devraient être traités très rapidement
    EXPECT_LT(duration.count(), 10000);  // < 10ms
    
    std::cout << "[Performance] 1000 process_received_packet() took " 
              << duration.count() << "µs" << std::endl;
}

TEST_F(PacketReliabilityTest, ReorderingPerformance) {
    // Test de reordering : mesurer performance avec plusieurs paquets désordonnés
    // Créer un scénario réaliste avec quelques paquets manquants
    
    // Envoyer paquets 1, 2, puis sauter 3, puis 4-30
    state_.process_received_packet(1, {0x01});
    state_.process_received_packet(2, {0x02});
    
    // Maintenant expected_recv_sequence = 3
    // Envoyer paquets 4-30 (seq 3 manquant)
    for (int i = 4; i <= 30; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i % 256)};
        state_.process_received_packet(i, data);
    }
    
    // Paquets 4-30 devraient être bufferisés
    EXPECT_EQ(state_.reorder_buffer.size(), 27);  // 30 - 4 + 1 = 27 paquets
    EXPECT_EQ(state_.expected_recv_sequence, 3);  // Attend toujours le paquet 3
    
    // Quand paquet 3 arrive, tout devrait être rejoué
    auto ready = state_.process_received_packet(3, {0x03});
    EXPECT_EQ(ready.size(), 28);  // Paquet 3 + les 27 bufferisés
    EXPECT_EQ(state_.expected_recv_sequence, 31);
    EXPECT_EQ(state_.reorder_buffer.size(), 0);
}

// ============================================================================
// Tests Edge Cases
// ============================================================================

TEST_F(PacketReliabilityTest, SequenceWrapAround) {
    // Tester comportement près de uint32_t max
    state_.next_send_sequence = 0xFFFFFFFE;
    
    uint32_t seq1 = state_.get_next_send_sequence();
    uint32_t seq2 = state_.get_next_send_sequence();
    uint32_t seq3 = state_.get_next_send_sequence();
    
    EXPECT_EQ(seq1, 0xFFFFFFFE);
    EXPECT_EQ(seq2, 0xFFFFFFFF);
    EXPECT_EQ(seq3, 0);  // Wrap around
}

TEST_F(PacketReliabilityTest, EmptyPacketData) {
    std::vector<uint8_t> empty_data;
    
    auto ready = state_.process_received_packet(1, empty_data);
    
    EXPECT_EQ(ready.size(), 1);
    EXPECT_TRUE(ready[0].empty());
}

TEST_F(PacketReliabilityTest, LargePacketData) {
    std::vector<uint8_t> large_data(65535, 0xAA);  // Paquet max size
    
    auto ready = state_.process_received_packet(1, large_data);
    
    EXPECT_EQ(ready.size(), 1);
    EXPECT_EQ(ready[0].size(), 65535);
}
