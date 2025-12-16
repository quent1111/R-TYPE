#pragma once

#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "network/UDPServer.hpp"

#include <unordered_map>

namespace server {

class LobbyBroadcaster {
public:
    LobbyBroadcaster();
    ~LobbyBroadcaster() = default;

    void broadcast_lobby_status(UDPServer& server,
                                const std::unordered_map<int, bool>& client_ready_status);

private:
    RType::BinarySerializer broadcast_serializer_;
};

}  // namespace server
