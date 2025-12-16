#pragma once

#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "network/UDPServer.hpp"

namespace server {

class GameBroadcaster {
public:
    GameBroadcaster() = default;
    ~GameBroadcaster() = default;

    void broadcast_level_info(UDPServer& server, registry& reg);
    void broadcast_level_complete(UDPServer& server, registry& reg);
    void broadcast_level_start(UDPServer& server, uint8_t level);
    void broadcast_boss_spawn(UDPServer& server);
    void broadcast_start_game(UDPServer& server);
    void broadcast_game_over(UDPServer& server);
};

}  // namespace server
