#include "network/EntityBroadcaster.hpp"

namespace server {

EntityBroadcaster::EntityBroadcaster() {
    broadcast_serializer_.reserve(65536);
}

void EntityBroadcaster::broadcast_entity_positions(
    UDPServer& server, registry& reg,
    const std::unordered_map<int, std::size_t>& client_entity_ids,
    const std::vector<int>& lobby_client_ids) {
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

            // Send health for boss and serpent parts
            if (tags[i]->type == RType::EntityType::Boss ||
                tags[i]->type == RType::EntityType::SerpentHead ||
                tags[i]->type == RType::EntityType::SerpentBody ||
                tags[i]->type == RType::EntityType::SerpentScale ||
                tags[i]->type == RType::EntityType::SerpentTail) {
                auto health_opt = reg.get_component<health>(entity_obj);
                int current_health = health_opt.has_value() ? health_opt->current : 100;
                int max_health = health_opt.has_value() ? health_opt->maximum : 100;
                broadcast_serializer_ << static_cast<int32_t>(current_health);
                broadcast_serializer_ << static_cast<int32_t>(max_health);
                
                // Send grayscale flag for serpent parts
                auto sprite_opt = reg.get_component<sprite_component>(entity_obj);
                bool grayscale = sprite_opt.has_value() ? sprite_opt->grayscale : false;
                broadcast_serializer_ << static_cast<uint8_t>(grayscale ? 1 : 0);
                
                // Send rotation for serpent parts (head, body, tail follow movement, scale aims at player)
                if (tags[i]->type == RType::EntityType::SerpentHead ||
                    tags[i]->type == RType::EntityType::SerpentBody ||
                    tags[i]->type == RType::EntityType::SerpentScale ||
                    tags[i]->type == RType::EntityType::SerpentTail) {
                    auto part_opt = reg.get_component<serpent_part>(entity_obj);
                    float rotation = part_opt.has_value() ? part_opt->rotation : 0.0f;
                    broadcast_serializer_ << rotation;
                    
                    // For scales, also send the body entity they're attached to
                    if (tags[i]->type == RType::EntityType::SerpentScale && part_opt.has_value()) {
                        uint32_t attached_id = 0;
                        if (part_opt->attached_body.has_value()) {
                            attached_id = OTHER_ID_OFFSET + static_cast<uint32_t>(
                                static_cast<std::size_t>(part_opt->attached_body.value()));
                        }
                        broadcast_serializer_ << attached_id;
                    }
                }
            }

            entity_count++;
        }
    }

    if (entity_count == 0)
        return;

    broadcast_serializer_.data()[count_position] = static_cast<uint8_t>(entity_count);
    server.send_to_clients(lobby_client_ids, broadcast_serializer_.data());
}

}  // namespace server
