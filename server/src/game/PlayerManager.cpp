#include "game/PlayerManager.hpp"

namespace server {

entity PlayerManager::create_player(registry& reg,
                                    std::unordered_map<int, std::size_t>& client_entity_ids,
                                    int client_id, float start_x, float start_y, int player_index) {
    if (client_entity_ids.find(client_id) != client_entity_ids.end()) {
        return reg.entity_from_index(client_entity_ids[client_id]);
    }

    auto player = ::createPlayer(reg, start_x, start_y, player_index);
    reg.emplace_component<network_id>(player, client_id);

    client_entity_ids[client_id] = player.id();
    std::cout << "[Game] Player created for client " << client_id << " with color index " << player_index 
              << " (Entity ID: " << player.id() << ")" << std::endl;
    return player;
}

std::optional<entity> PlayerManager::get_player_entity(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id) {
    auto it = client_entity_ids.find(client_id);
    if (it != client_entity_ids.end()) {
        return reg.entity_from_index(it->second);
    }
    return std::nullopt;
}

void PlayerManager::remove_player(registry& reg,
                                  std::unordered_map<int, std::size_t>& client_entity_ids,
                                  int client_id) {
    auto it = client_entity_ids.find(client_id);
    if (it != client_entity_ids.end()) {
        auto player = reg.entity_from_index(it->second);
        reg.kill_entity(player);
        client_entity_ids.erase(it);
        std::cout << "[Game] Player removed for client " << client_id << std::endl;
    }
}

bool PlayerManager::check_all_players_dead(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids) {
    if (client_entity_ids.empty()) {
        return false;
    }

    int alive_players = 0;
    for (const auto& [client_id, entity_id] : client_entity_ids) {
        auto player = reg.entity_from_index(entity_id);
        auto health_opt = reg.get_component<health>(player);

        if (health_opt.has_value() && health_opt->current > 0) {
            alive_players++;
        }
    }

    return alive_players == 0;
}

void PlayerManager::respawn_dead_players(registry& reg,
                                         std::unordered_map<int, std::size_t>& client_entity_ids) {
    for (const auto& [client_id, entity_id] : client_entity_ids) {
        auto player = reg.entity_from_index(entity_id);
        
        auto player_tag_opt = reg.get_component<player_tag>(player);
        if (!player_tag_opt.has_value()) {
            continue;
        }
        
        auto health_opt = reg.get_component<health>(player);

        if (health_opt.has_value() && health_opt->current <= 0) {
            auto& pos_opt = reg.get_component<position>(player);
            if (pos_opt.has_value()) {
                float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
                float start_y = 300.0f;
                pos_opt->x = start_x;
                pos_opt->y = start_y;
            }
            
            auto& vel_opt = reg.get_component<velocity>(player);
            if (vel_opt.has_value()) {
                vel_opt->vx = 0.0f;
                vel_opt->vy = 0.0f;
            }
            
            auto& controllable_opt = reg.get_component<controllable>(player);
            if (controllable_opt.has_value()) {
                controllable_opt->speed = 300.0f;
            }
            
            auto& collision_opt = reg.get_component<collision_box>(player);
            if (collision_opt.has_value()) {
                collision_opt->enabled = true;
            }
            
            auto& sprite_opt = reg.get_component<sprite_component>(player);
            if (sprite_opt.has_value()) {
                sprite_opt->visible = true;
            }
            
            auto& all_healths = reg.get_components<health>();
            auto& player_health_ref = all_healths[entity_id];
            if (player_health_ref.has_value()) {
                player_health_ref->current = player_health_ref->maximum;
            }
        }
    }
}

}  // namespace server
