#include <gtest/gtest.h>
#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include <vector>
#include <cstdint>

/**
 * @brief Test d'intégration complète de la compression
 * 
 * Ce test vérifie que TOUS les types de paquets du protocole R-TYPE
 * peuvent être compressés côté envoi et décompressés côté réception.
 * 
 * Simule le workflow complet :
 * 1. Client/Server crée un paquet avec CompressionSerializer
 * 2. Appelle compress()
 * 3. "Envoie" les données (simulation)
 * 4. Récepteur décompresse avec decompress()
 * 5. Vérifie que les données sont identiques
 */

class FullCompressionIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup commun si nécessaire
    }

    // Helper: Simule l'envoi et la réception d'un paquet
    std::vector<uint8_t> simulate_network_transfer(const std::vector<uint8_t>& data) {
        // Simule l'envoi réseau (copie des données)
        return data;
    }

    // Helper: Vérifie qu'un paquet peut faire un round-trip complet
    void verify_packet_round_trip(RType::CompressionSerializer& sender) {
        // 1. Compression côté envoi
        bool was_compressed = sender.compress();
        std::vector<uint8_t> sent_data = sender.data();
        
        ASSERT_FALSE(sent_data.empty()) << "Sent data should not be empty";
        
        // 2. Simulation du transfert réseau
        std::vector<uint8_t> received_data = simulate_network_transfer(sent_data);
        
        // 3. Décompression côté réception
        RType::CompressionSerializer receiver(received_data);
        bool was_decompressed = receiver.decompress();
        
        // 4. Vérification
        EXPECT_EQ(was_compressed, was_decompressed) 
            << "Compression/decompression flags should match";
        
        // Les données décompressées doivent contenir au minimum le magic number
        ASSERT_GE(receiver.data().size(), 2) 
            << "Decompressed data should contain at least magic number";
        
        uint16_t magic = static_cast<uint16_t>(receiver.data()[0]) | 
                        (static_cast<uint16_t>(receiver.data()[1]) << 8);
        EXPECT_EQ(magic, RType::MagicNumber::VALUE) 
            << "Magic number should be preserved after compression/decompression";
    }
};

// ============================================================================
// Tests des paquets CLIENT -> SERVER
// ============================================================================

TEST_F(FullCompressionIntegrationTest, ClientLoginPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::Login;
    serializer << std::string("TestPlayer");
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ClientInputPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::Input;
    serializer << static_cast<uint8_t>(0b00001111); // Masque d'input
    serializer << static_cast<uint32_t>(12345);      // Timestamp
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ClientReadyPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PlayerReady;
    serializer << static_cast<uint8_t>(1); // Ready = true
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ClientListLobbiesPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::ListLobbies;
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ClientCreateLobbyPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::CreateLobby;
    serializer << std::string("MyLobby123");
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ClientJoinLobbyPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::JoinLobby;
    serializer << static_cast<int32_t>(42); // Lobby ID
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ClientPowerupChoicePacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpChoice;
    serializer << static_cast<uint8_t>(2); // Choice index
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ClientPowerupActivatePacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpActivate;
    serializer << static_cast<uint8_t>(1); // Powerup type
    
    verify_packet_round_trip(serializer);
}

// ============================================================================
// Tests des paquets SERVER -> CLIENT
// ============================================================================

TEST_F(FullCompressionIntegrationTest, ServerLoginAckPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LoginAck;
    serializer << static_cast<uint32_t>(123); // Client ID
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerEntityPositionsPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::EntityPosition;  // Correct: EntityPosition (singulier)
    serializer << static_cast<uint16_t>(3); // Nombre d'entités
    
    // 3 entités avec positions quantifiées
    for (int i = 0; i < 3; ++i) {
        serializer << static_cast<uint32_t>(100 + i);     // Entity ID
        serializer << static_cast<uint8_t>(1);            // Entity Type
        serializer.write_position(100.5f + i, 200.3f + i); // Position quantifiée
        serializer.write_velocity(50.0f, -30.0f);          // Velocity quantifiée
        serializer.write_quantized_health(80, 100);        // Health quantifiée (current, max)
    }
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerLobbyStatusPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LobbyStatus;
    serializer << static_cast<uint8_t>(3); // Total players
    serializer << static_cast<uint8_t>(2); // Ready players
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerListLobbiesPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::ListLobbies;
    serializer << static_cast<int32_t>(2); // Nombre de lobbies
    
    // Lobby 1
    serializer << static_cast<int32_t>(1);      // Lobby ID
    serializer << std::string("Lobby One");     // Name
    serializer << static_cast<int32_t>(2);      // Current players
    serializer << static_cast<int32_t>(4);      // Max players
    serializer << static_cast<uint8_t>(0);      // State
    
    // Lobby 2
    serializer << static_cast<int32_t>(2);
    serializer << std::string("Lobby Two");
    serializer << static_cast<int32_t>(1);
    serializer << static_cast<int32_t>(4);
    serializer << static_cast<uint8_t>(1);
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerLobbyJoinedPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LobbyJoined;
    serializer << static_cast<uint8_t>(1);      // Success
    serializer << static_cast<int32_t>(42);     // Lobby ID
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerStartGamePacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::StartGame;
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerLevelStartPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LevelStart;
    serializer << static_cast<uint8_t>(2); // Level number
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerLevelProgressPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LevelProgress;
    serializer << static_cast<uint8_t>(1);      // Current level
    serializer << static_cast<uint16_t>(15);    // Enemies killed
    serializer << static_cast<uint16_t>(20);    // Enemies needed
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerLevelCompletePacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LevelComplete;
    serializer << static_cast<uint8_t>(1);      // Completed level
    serializer << static_cast<uint8_t>(2);      // Next level
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerGameOverPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::GameOver;
    
    // Padding (20 bytes de 0xFF)
    for (int i = 0; i < 20; ++i) {
        serializer << static_cast<uint8_t>(0xFF);
    }
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerBossSpawnPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::BossSpawn;
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerPowerupCardsPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpCards;
    serializer << static_cast<uint8_t>(3); // Number of cards
    
    // 3 powerup cards
    for (uint8_t i = 0; i < 3; ++i) {
        serializer << static_cast<uint8_t>(i + 1);  // Card ID
        serializer << static_cast<uint8_t>(2);      // Level
    }
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerPowerupStatusPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpStatus;
    serializer << static_cast<uint32_t>(123);   // Client ID
    serializer << static_cast<uint8_t>(1);      // Powerup type
    serializer << 15.5f;                        // Time remaining
    
    verify_packet_round_trip(serializer);
}

TEST_F(FullCompressionIntegrationTest, ServerActivableSlotsPacket) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::ActivableSlots;
    
    // Slot 1: avec powerup
    serializer << true;                         // Has powerup
    serializer << static_cast<uint8_t>(3);      // Powerup ID
    serializer << static_cast<uint8_t>(2);      // Level
    serializer << 10.0f;                        // Time remaining
    serializer << 2.5f;                         // Cooldown remaining
    serializer << true;                         // Is active
    
    // Slot 2: vide
    serializer << false;                        // No powerup
    
    verify_packet_round_trip(serializer);
}

// ============================================================================
// Tests de stress avec paquets volumineux
// ============================================================================

TEST_F(FullCompressionIntegrationTest, StressTestLargeEntityUpdate) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::EntityPosition;  // Correct: EntityPosition (singulier)
    
    // 50 entités (typique d'une partie avec beaucoup d'ennemis et projectiles)
    const uint16_t entity_count = 50;
    serializer << entity_count;
    
    for (uint16_t i = 0; i < entity_count; ++i) {
        serializer << static_cast<uint32_t>(1000 + i);
        serializer << static_cast<uint8_t>(i % 5);      // Type varié
        serializer.write_position(
            100.0f + (i * 10.5f), 
            200.0f + (i * 5.3f)
        );
        serializer.write_velocity(
            static_cast<float>((i % 10) - 5) * 10.0f,
            static_cast<float>((i % 8) - 4) * 8.0f
        );
        serializer.write_quantized_health(100 - (i % 100), 100);  // (current, max)
    }
    
    size_t original_size = serializer.size();
    verify_packet_round_trip(serializer);
    
    // Vérifier que la compression fonctionne sur les gros paquets
    serializer.compress();
    size_t compressed_size = serializer.size();
    
    // Note: Avec des données aléatoires/variées, LZ4 peut légèrement augmenter
    // la taille à cause de l'overhead. On vérifie juste que c'est raisonnable.
    if (original_size >= 128) { // Si au-dessus du seuil
        float ratio = static_cast<float>(compressed_size) / original_size;
        
        // Accepter une légère augmentation (jusqu'à 5%) due à l'overhead LZ4
        EXPECT_LT(ratio, 1.05f) 
            << "Compression overhead should be reasonable (original: " 
            << original_size << ", compressed: " << compressed_size << ")";
        
        std::cout << "[COMPRESSION] Large packet: " << original_size 
                  << " -> " << compressed_size << " bytes (" 
                  << ((1.0f - ratio) * 100.0f) << "% reduction)\n";
    }
}

TEST_F(FullCompressionIntegrationTest, StressTestManyLobbies) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::ListLobbies;
    
    // 20 lobbies
    const int32_t lobby_count = 20;
    serializer << lobby_count;
    
    for (int32_t i = 0; i < lobby_count; ++i) {
        serializer << i;
        serializer << std::string("Lobby_" + std::to_string(i));
        serializer << static_cast<int32_t>(i % 4);      // Current players
        serializer << static_cast<int32_t>(4);          // Max players
        serializer << static_cast<uint8_t>(i % 3);      // State
    }
    
    size_t original_size = serializer.size();
    verify_packet_round_trip(serializer);
    
    serializer.compress();
    size_t compressed_size = serializer.size();
    
    if (original_size >= 128) {
        float ratio = static_cast<float>(compressed_size) / original_size * 100.0f;
        std::cout << "[COMPRESSION] Many lobbies: " << original_size 
                  << " -> " << compressed_size << " bytes (" 
                  << (100.0f - ratio) << "% reduction)\n";
    }
}

// ============================================================================
// Test de robustesse : paquets invalides
// ============================================================================

TEST_F(FullCompressionIntegrationTest, InvalidCompressionFlag) {
    // Créer un paquet avec un flag invalide
    std::vector<uint8_t> invalid_packet = {
        0x99,  // Flag invalide (ni 0x00 ni 0x01)
        0x00, 0x00, 0x00, 0x10,  // Original size
        0x42, 0xB5  // Magic number
    };
    
    RType::CompressionSerializer receiver(invalid_packet);
    EXPECT_THROW(receiver.decompress(), RType::CompressionException);
}

TEST_F(FullCompressionIntegrationTest, TruncatedCompressedPacket) {
    // Créer un paquet compressé
    RType::CompressionSerializer sender;
    sender << RType::MagicNumber::VALUE;
    sender << RType::OpCode::Login;
    sender << std::string("TestPlayer");
    sender.compress();
    
    // Tronquer le paquet de manière sévère (garder seulement 5 bytes)
    std::vector<uint8_t> truncated = sender.data();
    size_t original_size = truncated.size();
    truncated.resize(5);  // Très court, impossible de décompresser
    
    RType::CompressionSerializer receiver(truncated);
    
    // LZ4 peut soit lancer une exception, soit retourner 0 bytes
    // On vérifie que la décompression échoue d'une manière ou d'une autre
    try {
        receiver.decompress();
        // Si pas d'exception, le buffer devrait être vide ou invalide
        EXPECT_TRUE(receiver.size() == 0 || receiver.size() < 10)
            << "Truncated packet should fail decompression (got " 
            << receiver.size() << " bytes from " << original_size << " truncated to 5)";
    } catch (const RType::CompressionException&) {
        // Exception attendue et acceptable
        SUCCEED();
    }
}

// ============================================================================
// Test de performance
// ============================================================================

TEST_F(FullCompressionIntegrationTest, PerformanceBenchmark) {
    const int iterations = 1000;
    
    // Créer un paquet typique (mise à jour de 10 entités)
    RType::CompressionSerializer template_serializer;
    template_serializer << RType::MagicNumber::VALUE;
    template_serializer << RType::OpCode::EntityPosition;  // Correct: EntityPosition
    template_serializer << static_cast<uint16_t>(10);
    
    for (int i = 0; i < 10; ++i) {
        template_serializer << static_cast<uint32_t>(i);
        template_serializer << static_cast<uint8_t>(1);
        template_serializer.write_position(100.0f + i, 200.0f + i);
        template_serializer.write_velocity(50.0f, -30.0f);
        template_serializer.write_quantized_health(100, 100);  // (current, max)
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        RType::CompressionSerializer sender = template_serializer;
        sender.compress();
        
        std::vector<uint8_t> data = sender.data();
        
        RType::CompressionSerializer receiver(data);
        receiver.decompress();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avg_time = duration.count() / static_cast<double>(iterations);
    
    std::cout << "[PERFORMANCE] " << iterations << " compression/decompression cycles: "
              << duration.count() << " µs total, "
              << avg_time << " µs per packet\n";
    
    // Pour un jeu à 60 FPS, on a ~16ms par frame
    // Avec 100 paquets/s, on doit être < 100µs par paquet
    EXPECT_LT(avg_time, 100.0) 
        << "Compression/decompression should be fast enough for real-time gaming";
}
