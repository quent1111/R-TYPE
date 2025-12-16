#include "network/EntityBroadcaster.hpp"

namespace server {

EntityBroadcaster::EntityBroadcaster() {
    broadcast_serializer_.reserve(65536);
}

void EntityBroadcaster::broadcast_entity_positions(
    UDPServer& server, registry& reg,
    const std::unordered_map<int, std::size_t>& client_entity_ids) {
    broadcast_serializer_.clear();

    broadcast_serializer_ << RType::MagicNumber::VALUE;
    broadcast_serializer_ << RType::OpCode::EntityPosition;

    size_t count_position = broadcast_serializer_.data().size();
    broadcast_serializer_ << static_cast<uint8_t>(0);

    size_t entity_count = 0;
    auto& tags = reg.get_components<entity_tag>();
    auto& positions = reg.get_components<position>();

    for (const auto& [client_id, entity_id] : client_entity_ids) {
        auto player = reg.entity_from_index(entity_id);
        auto pos_opt = reg.get_component<position>(player);
        auto vel_opt = reg.get_component<velocity>(player);
        auto health_opt = reg.get_component<health>(player);

        if (pos_opt.has_value()) {
            const auto& pos = pos_opt.value();
            broadcast_serializer_ << static_cast<uint32_t>(client_id);
            broadcast_serializer_ << static_cast<uint8_t>(RType::EntityType::Player);
            broadcast_serializer_ << pos.x;
            broadcast_serializer_ << pos.y;
            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            broadcast_serializer_ << vx;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            broadcast_serializer_ << vy;

            int current_health = health_opt.has_value() ? health_opt->current : 100;
            int max_health = health_opt.has_value() ? health_opt->maximum : 100;
            broadcast_serializer_ << static_cast<int32_t>(current_health);
            broadcast_serializer_ << static_cast<int32_t>(max_health);

            entity_count++;
        }
    }

    for (size_t i = 0; i < tags.size(); ++i) {
        if (!tags[i].has_value())
            continue;
        if (i >= positions.size() || !positions[i].has_value())
            continue;

        if (tags[i]->type != RType::EntityType::Player) {
            const auto& pos = positions[i].value();
            auto entity_obj = reg.entity_from_index(i);
            auto vel_opt = reg.get_component<velocity>(entity_obj);

            uint32_t network_id;
            if (tags[i]->type == RType::EntityType::Enemy ||
                tags[i]->type == RType::EntityType::Enemy2) {
                network_id = ENEMY_ID_OFFSET + static_cast<uint32_t>(i);
            } else if (tags[i]->type == RType::EntityType::Boss) {
                network_id = ENEMY_ID_OFFSET + static_cast<uint32_t>(i);
            } else if (tags[i]->type == RType::EntityType::Projectile) {
                network_id = PROJECTILE_ID_OFFSET + static_cast<uint32_t>(i);
            } else {
                network_id = OTHER_ID_OFFSET + static_cast<uint32_t>(i);
            }

            broadcast_serializer_ << network_id;
            broadcast_serializer_ << static_cast<uint8_t>(tags[i]->type);
            broadcast_serializer_ << pos.x;
            broadcast_serializer_ << pos.y;
            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            broadcast_serializer_ << vx;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            broadcast_serializer_ << vy;
            entity_count++;
        }
    }

    if (entity_count == 0)
        return;

    broadcast_serializer_.data()[count_position] = static_cast<uint8_t>(entity_count);
    server.send_to_all(broadcast_serializer_.data());
}

}  // namespace server
