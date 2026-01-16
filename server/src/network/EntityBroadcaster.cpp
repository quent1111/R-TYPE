#include "network/EntityBroadcaster.hpp"

#include "../../src/Common/CompressionSerializer.hpp"

#include <iostream>

namespace server {

EntityBroadcaster::EntityBroadcaster() {
    broadcast_serializer_.reserve(65536);

    // Configure compression
    RType::CompressionConfig config;
    config.min_compress_size = 128;       // Compress packets >= 128 bytes
    config.acceleration = 10;             // Balance between speed and ratio
    config.use_high_compression = false;  // Fast mode for real-time
    broadcast_serializer_.set_config(config);
}

void EntityBroadcaster::broadcast_entity_positions(
    UDPServer& server, registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids,
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
        auto player_idx_opt = reg.get_component<player_index_component>(player);

        if (pos_opt.has_value()) {
            const auto& pos = pos_opt.value();

            broadcast_serializer_ << static_cast<uint32_t>(client_id);
            broadcast_serializer_ << static_cast<uint8_t>(RType::EntityType::Player);

            uint8_t player_idx = player_idx_opt.has_value()
                                     ? static_cast<uint8_t>(player_idx_opt->index)
                                     : uint8_t{0};
            broadcast_serializer_ << player_idx;

            broadcast_serializer_.write_position(pos.x, pos.y);

            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            broadcast_serializer_.write_velocity(vx, vy);

            int current_health = health_opt.has_value() ? health_opt->current : 100;
            int max_health = health_opt.has_value() ? health_opt->maximum : 100;
            broadcast_serializer_.write_quantized_health(current_health, max_health);

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
            } else if (tags[i]->type == RType::EntityType::CustomEnemy) {
                network_id = ENEMY_ID_OFFSET + static_cast<uint32_t>(i);
            } else if (tags[i]->type == RType::EntityType::CustomBoss) {
                network_id = ENEMY_ID_OFFSET + static_cast<uint32_t>(i);
            } else if (tags[i]->type == RType::EntityType::Projectile) {
                network_id = PROJECTILE_ID_OFFSET + static_cast<uint32_t>(i);
            } else if (tags[i]->type == RType::EntityType::CustomProjectile) {
                network_id = PROJECTILE_ID_OFFSET + static_cast<uint32_t>(i);
            } else {
                network_id = OTHER_ID_OFFSET + static_cast<uint32_t>(i);
            }

            broadcast_serializer_ << network_id;
            broadcast_serializer_ << static_cast<uint8_t>(tags[i]->type);

            broadcast_serializer_.write_position(pos.x, pos.y);

            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            broadcast_serializer_.write_velocity(vx, vy);

            // Send custom entity ID for custom entities (enemy, boss, projectile)
            if (tags[i]->type == RType::EntityType::CustomEnemy ||
                tags[i]->type == RType::EntityType::CustomBoss ||
                tags[i]->type == RType::EntityType::CustomProjectile) {
                auto custom_id_opt = reg.get_component<custom_entity_id>(entity_obj);
                std::string entity_id_str =
                    custom_id_opt.has_value() ? custom_id_opt->entity_id : "";

                // Send string length (uint8_t) then string data
                uint8_t id_length =
                    static_cast<uint8_t>(std::min(entity_id_str.length(), size_t(255)));
                broadcast_serializer_ << id_length;
                for (uint8_t j = 0; j < id_length; ++j) {
                    broadcast_serializer_ << static_cast<uint8_t>(entity_id_str[j]);
                }
            }

            // Send health for boss and serpent parts
            if (tags[i]->type == RType::EntityType::Boss ||
                tags[i]->type == RType::EntityType::CustomBoss ||
                tags[i]->type == RType::EntityType::SerpentHead ||
                tags[i]->type == RType::EntityType::SerpentBody ||
                tags[i]->type == RType::EntityType::SerpentScale ||
                tags[i]->type == RType::EntityType::SerpentTail ||
                tags[i]->type == RType::EntityType::CompilerPart1 ||
                tags[i]->type == RType::EntityType::CompilerPart2 ||
                tags[i]->type == RType::EntityType::CompilerPart3) {
                auto health_opt = reg.get_component<health>(entity_obj);
                int current_health = health_opt.has_value() ? health_opt->current : 100;
                int max_health = health_opt.has_value() ? health_opt->maximum : 100;
                broadcast_serializer_.write_quantized_health(current_health, max_health);

                // Send grayscale flag for serpent parts
                auto sprite_opt = reg.get_component<sprite_component>(entity_obj);
                bool grayscale = sprite_opt.has_value() ? sprite_opt->grayscale : false;
                broadcast_serializer_ << static_cast<uint8_t>(grayscale ? 1 : 0);

                // Send rotation for serpent parts (head, body, tail follow movement, scale aims at
                // player)
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
                            attached_id =
                                OTHER_ID_OFFSET + static_cast<uint32_t>(static_cast<std::size_t>(
                                                      part_opt->attached_body.value()));
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

    broadcast_serializer_.compress();

    server.send_to_clients(lobby_client_ids, broadcast_serializer_.data());
}

void EntityBroadcaster::send_full_game_state_to_client(
    UDPServer& server, registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids,
    int client_id) {
    std::cout << "[EntityBroadcaster] Sending full game state to client " << client_id << std::endl;

    RType::BinarySerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << RType::OpCode::EntityPosition;

    size_t count_position = serializer.data().size();
    serializer << static_cast<uint8_t>(0);

    size_t entity_count = 0;
    auto& tags = reg.get_components<entity_tag>();
    auto& positions = reg.get_components<position>();

    for (const auto& [other_client_id, entity_id] : client_entity_ids) {
        auto player = reg.entity_from_index(entity_id);
        auto pos_opt = reg.get_component<position>(player);
        auto vel_opt = reg.get_component<velocity>(player);
        auto health_opt = reg.get_component<health>(player);
        auto player_idx_opt = reg.get_component<player_index_component>(player);

        if (pos_opt.has_value()) {
            const auto& pos = pos_opt.value();
            serializer << static_cast<uint32_t>(other_client_id);
            serializer << static_cast<uint8_t>(RType::EntityType::Player);

            uint8_t player_idx = player_idx_opt.has_value()
                                     ? static_cast<uint8_t>(player_idx_opt->index)
                                     : uint8_t{0};
            serializer << player_idx;

            serializer << pos.x;
            serializer << pos.y;
            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            serializer << vx;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            serializer << vy;

            int current_health = health_opt.has_value() ? health_opt->current : 100;
            int max_health = health_opt.has_value() ? health_opt->maximum : 100;
            serializer << static_cast<int32_t>(current_health);
            serializer << static_cast<int32_t>(max_health);

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

            serializer << network_id;
            serializer << static_cast<uint8_t>(tags[i]->type);
            serializer << pos.x;
            serializer << pos.y;
            float vx = vel_opt.has_value() ? vel_opt->vx : 0.0f;
            serializer << vx;
            float vy = vel_opt.has_value() ? vel_opt->vy : 0.0f;
            serializer << vy;

            if (tags[i]->type == RType::EntityType::Boss) {
                auto health_opt = reg.get_component<health>(entity_obj);
                int current_health = health_opt.has_value() ? health_opt->current : 100;
                int max_health = health_opt.has_value() ? health_opt->maximum : 100;
                serializer << static_cast<int32_t>(current_health);
                serializer << static_cast<int32_t>(max_health);
            }

            entity_count++;
        }
    }

    if (entity_count == 0) {
        std::cout << "[EntityBroadcaster] No entities to send to client " << client_id << std::endl;
        return;
    }

    serializer.data()[count_position] = static_cast<uint8_t>(entity_count);

    std::cout << "[EntityBroadcaster] Sending " << entity_count << " entities to client "
              << client_id << std::endl;
    server.send_to_client(client_id, serializer.data());
}

void EntityBroadcaster::print_compression_stats() const {
    const auto& stats = broadcast_serializer_.get_stats();
    std::cout << "\n=== EntityBroadcaster Compression Stats ===" << std::endl;
    std::cout << "  Compressed packets   : " << stats.total_compressed << std::endl;
    std::cout << "  Uncompressed packets : " << stats.total_uncompressed << std::endl;
    std::cout << "  Total bytes in       : " << stats.total_bytes_in << " bytes" << std::endl;
    std::cout << "  Total bytes out      : " << stats.total_bytes_out << " bytes" << std::endl;
    std::cout << "  Compression ratio    : " << (stats.get_compression_ratio() * 100.0) << "%"
              << std::endl;
    std::cout << "  Bandwidth savings    : " << stats.get_savings_percent() << "%" << std::endl;
    std::cout << "==========================================\n" << std::endl;
}

}  // namespace server
