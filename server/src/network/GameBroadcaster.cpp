#include "network/GameBroadcaster.hpp"

#include <iostream>

namespace server {

void GameBroadcaster::broadcast_level_info(UDPServer& server, registry& reg, const std::vector<int>& lobby_client_ids) {
    auto& level_managers = reg.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            RType::BinarySerializer serializer;
            serializer << RType::MagicNumber::VALUE;
            serializer << RType::OpCode::LevelProgress;
            serializer << static_cast<uint8_t>(lvl_mgr.current_level);
            serializer << static_cast<uint16_t>(lvl_mgr.enemies_killed_this_level);
            serializer << static_cast<uint16_t>(lvl_mgr.enemies_needed_for_next_level);
            server.send_to_clients(lobby_client_ids, serializer.data());
            break;
        }
    }
}

void GameBroadcaster::broadcast_level_complete(UDPServer& server, registry& reg, const std::vector<int>& lobby_client_ids) {
    auto& level_managers = reg.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            RType::BinarySerializer serializer;
            serializer << RType::MagicNumber::VALUE;
            serializer << RType::OpCode::LevelComplete;
            serializer << static_cast<uint8_t>(lvl_mgr.current_level);
            serializer << static_cast<uint8_t>(lvl_mgr.current_level + 1);
            server.send_to_clients(lobby_client_ids, serializer.data());
            break;
        }
    }
}

void GameBroadcaster::broadcast_level_start(UDPServer& server, uint8_t level, const std::vector<int>& lobby_client_ids) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::LevelStart;
    serializer << level;
    server.send_to_clients(lobby_client_ids, serializer.data());
    std::cout << "[Game] Sent Level " << static_cast<int>(level)
              << " start screen (opcode: 0x30, level: " << static_cast<int>(level) << ")"
              << std::endl;
}

void GameBroadcaster::broadcast_boss_spawn(UDPServer& server, const std::vector<int>& lobby_client_ids) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::BossSpawn;
    server.send_to_clients(lobby_client_ids, serializer.data());
    std::cout << "[Game] Broadcasting Boss Spawn (opcode 0x50) - Music & Roar trigger" << std::endl;
}

void GameBroadcaster::broadcast_start_game(UDPServer& server, const std::vector<int>& lobby_client_ids) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::StartGame;
    server.send_to_clients(lobby_client_ids, serializer.data());
}

void GameBroadcaster::broadcast_game_over(UDPServer& server, const std::vector<int>& lobby_client_ids) {
    std::cout << "[Game] Broadcasting GameOver (opcode 0x40) to lobby clients..." << std::endl;
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::GameOver;

    for (int i = 0; i < 20; ++i) {
        serializer << static_cast<uint8_t>(0xFF);
    }

    server.send_to_clients(lobby_client_ids, serializer.data());
}

}  // namespace server
