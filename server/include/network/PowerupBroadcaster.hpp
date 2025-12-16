#pragma once

#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "network/UDPServer.hpp"

#include <unordered_map>

namespace server {

class PowerupBroadcaster {
public:
    PowerupBroadcaster() = default;
    ~PowerupBroadcaster() = default;

    void broadcast_powerup_selection(UDPServer& server);
    void broadcast_powerup_status(UDPServer& server, registry& reg,
                                  const std::unordered_map<int, std::size_t>& client_entity_ids);
};

}  // namespace server
