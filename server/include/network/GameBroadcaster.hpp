#pragma once

#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "network/UDPServer.hpp"

namespace server {

class GameBroadcaster {
public:
    GameBroadcaster() = default;
    ~GameBroadcaster() = default;

    void broadcast_level_info(UDPServer& server, registry& reg, const std::vector<int>& lobby_client_ids);
    void broadcast_level_complete(UDPServer& server, registry& reg, const std::vector<int>& lobby_client_ids);
    void broadcast_level_start(UDPServer& server, uint8_t level, const std::string& custom_level_id, const std::vector<int>& lobby_client_ids);
    void broadcast_boss_spawn(UDPServer& server, const std::vector<int>& lobby_client_ids);
    void broadcast_start_game(UDPServer& server, const std::vector<int>& lobby_client_ids);
    void broadcast_game_over(UDPServer& server, const std::vector<int>& lobby_client_ids);
};

}  // namespace server
