#include "network/PowerupBroadcaster.hpp"

namespace server {

void PowerupBroadcaster::broadcast_powerup_selection(UDPServer& server, const std::vector<int>& lobby_client_ids) {
    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::PowerUpChoice;
    serializer << static_cast<uint8_t>(1);
    server.send_to_clients(lobby_client_ids, serializer.data());
}

void PowerupBroadcaster::broadcast_powerup_status(
    UDPServer& server, registry& reg,
    const std::unordered_map<int, std::size_t>& client_entity_ids,
    const std::vector<int>& lobby_client_ids) {
    for (const auto& [client_id, entity_id] : client_entity_ids) {
        auto player = reg.entity_from_index(entity_id);

        auto& cannon_opt = reg.get_component<power_cannon>(player);
        if (cannon_opt.has_value()) {
            uint8_t powerup_type = 1;
            float time_remaining = 0.0f;
            if (cannon_opt->is_active()) {
                time_remaining = cannon_opt->time_remaining;
            }
            RType::BinarySerializer serializer;
            serializer << RType::MagicNumber::VALUE;
            serializer << RType::OpCode::PowerUpStatus;
            serializer << static_cast<uint32_t>(client_id);
            serializer << powerup_type;
            serializer << time_remaining;
            server.send_to_clients(lobby_client_ids, serializer.data());
        }

        auto& shield_opt = reg.get_component<shield>(player);
        if (shield_opt.has_value()) {
            uint8_t powerup_type = 2;
            float time_remaining = 0.0f;
            if (shield_opt->is_active()) {
                time_remaining = shield_opt->time_remaining;
            }
            RType::BinarySerializer serializer;
            serializer << RType::MagicNumber::VALUE;
            serializer << RType::OpCode::PowerUpStatus;
            serializer << static_cast<uint32_t>(client_id);
            serializer << powerup_type;
            serializer << time_remaining;
            server.send_to_clients(lobby_client_ids, serializer.data());
        }
    }
}

}  // namespace server
