#pragma once

#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "network/UDPServer.hpp"

#include <unordered_map>

namespace server {

class LobbyBroadcaster {
public:
    LobbyBroadcaster();
    ~LobbyBroadcaster() = default;

    void broadcast_lobby_status(UDPServer& server,
                                const std::unordered_map<int, bool>& client_ready_status,
                                const std::vector<int>& lobby_client_ids);

private:
    RType::CompressionSerializer broadcast_serializer_;
};

}  // namespace server
