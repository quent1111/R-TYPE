#pragma once

#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../game-lib/include/powerup/PowerupCardPool.hpp"
#include "../../game-lib/include/powerup/PowerupRegistry.hpp"
#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "network/UDPServer.hpp"

#include <unordered_map>
#include <vector>

namespace server {

class PowerupBroadcaster {
public:
    PowerupBroadcaster() = default;
    ~PowerupBroadcaster() = default;

    void broadcast_powerup_selection(UDPServer& server, const std::vector<int>& lobby_client_ids);

    void broadcast_powerup_cards(UDPServer& server, int client_id,
                                 const std::vector<powerup::PowerupCard>& cards);

    void broadcast_powerup_status(UDPServer& server, registry& reg,
                                  const std::unordered_map<int, std::size_t>& client_entity_ids,
                                  const std::vector<int>& lobby_client_ids);

    void broadcast_activable_slots(UDPServer& server, int client_id,
                                   const powerup::PlayerPowerups::ActivableSlot slots[2]);
};

}  // namespace server
