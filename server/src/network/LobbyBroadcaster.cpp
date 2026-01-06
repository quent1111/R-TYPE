#include "network/LobbyBroadcaster.hpp"

namespace server {

LobbyBroadcaster::LobbyBroadcaster() {
    broadcast_serializer_.reserve(1024);
}

void LobbyBroadcaster::broadcast_lobby_status(
    UDPServer& server, const std::unordered_map<int, bool>& client_ready_status,
    const std::vector<int>& lobby_client_ids) {
    broadcast_serializer_.clear();

    broadcast_serializer_ << RType::MagicNumber::VALUE;
    broadcast_serializer_ << RType::OpCode::LobbyStatus;

    uint8_t total_players = static_cast<uint8_t>(client_ready_status.size());
    uint8_t ready_players = 0;

    for (const auto& [client_id, ready] : client_ready_status) {
        if (ready)
            ready_players++;
    }

    broadcast_serializer_ << total_players;
    broadcast_serializer_ << ready_players;

    server.send_to_clients(lobby_client_ids, broadcast_serializer_.data());
}

}  // namespace server
