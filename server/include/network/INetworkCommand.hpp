#pragma once

#include "../../engine/ecs/registry.hpp"

#include <memory>
#include <unordered_map>

namespace server {

class UDPServer;

class INetworkCommand {
public:
    virtual ~INetworkCommand() = default;

    /**
     * @brief Execute the command.
     * @param reg ECS registry.
     * @param client_entity_ids Map of client IDs to entity indices.
     * @param server Reference to UDP server for sending responses.
     */
    virtual void execute(registry& reg, std::unordered_map<int, std::size_t>& client_entity_ids,
                         UDPServer& server) = 0;
};

}  // namespace server
