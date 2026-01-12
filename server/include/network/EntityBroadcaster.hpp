#pragma once

#include "../../engine/ecs/components.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "common/GameConstants.hpp"
#include "network/UDPServer.hpp"

#include <unordered_map>

namespace server {

class EntityBroadcaster {
public:
    EntityBroadcaster();
    ~EntityBroadcaster() = default;

    void broadcast_entity_positions(UDPServer& server, registry& reg,
                                    const std::unordered_map<int, std::size_t>& client_entity_ids,
                                    const std::vector<int>& lobby_client_ids);

    void send_full_game_state_to_client(UDPServer& server, registry& reg,
                                       const std::unordered_map<int, std::size_t>& client_entity_ids,
                                       int client_id);

private:
    RType::BinarySerializer broadcast_serializer_;
};

}  // namespace server
