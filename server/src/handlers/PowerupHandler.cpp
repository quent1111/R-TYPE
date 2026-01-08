#include "handlers/PowerupHandler.hpp"

namespace server {

std::optional<entity> PowerupHandler::get_player_entity(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id) {
    auto it = client_entity_ids.find(client_id);
    if (it != client_entity_ids.end()) {
        return reg.entity_from_index(it->second);
    }
    return std::nullopt;
}

int PowerupHandler::count_alive_players(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids) {
    int alive_players = 0;
    for (const auto& [cid, entity_id] : client_entity_ids) {
        auto p = reg.entity_from_index(entity_id);
        auto health_opt = reg.get_component<health>(p);
        if (health_opt.has_value() && health_opt->current > 0) {
            alive_players++;
        }
    }
    return alive_players;
}

void PowerupHandler::handle_powerup_choice(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids,
    std::unordered_set<int>& players_who_chose_powerup, int client_id, uint8_t /*powerup_choice*/) {
    auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
    if (!player_opt.has_value()) {
        std::cerr << "[Game] Cannot apply power-up: player not found for client " << client_id
                  << std::endl;
        return;
    }

    auto player = player_opt.value();

    reg.emplace_component<power_cannon>(player);
    reg.emplace_component<shield>(player);
    reg.emplace_component<little_friend>(player);
    std::cout << "[Game] Client " << client_id << " received Power Cannon, Shield and Little Friend"
              << std::endl;

    players_who_chose_powerup.insert(client_id);

    int alive_players = count_alive_players(reg, client_entity_ids);

    std::cout << "[Game] Powerup choices: " << players_who_chose_powerup.size() << "/"
              << alive_players << " players" << std::endl;
}

void PowerupHandler::handle_powerup_activate(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id,
    uint8_t powerup_type) {
    auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
    if (!player_opt.has_value()) {
        return;
    }
    auto player = player_opt.value();

    if (powerup_type == 1) {
        auto& cannon_opt = reg.get_component<power_cannon>(player);
        if (cannon_opt.has_value() && !cannon_opt->is_active()) {
            cannon_opt->activate();
            std::cout << "[Game] Client " << client_id << " activated Power Cannon" << std::endl;
        }
    } else if (powerup_type == 2) {
        auto& shield_opt = reg.get_component<shield>(player);
        if (shield_opt.has_value() && !shield_opt->is_active()) {
            shield_opt->activate();
            std::cout << "[Game] Client " << client_id << " activated Shield" << std::endl;
        }
    } else if (powerup_type == 3) {
        auto& friend_opt = reg.get_component<little_friend>(player);
        if (friend_opt.has_value() && !friend_opt->is_active()) {
            friend_opt->activate();
            std::cout << "[Game] Client " << client_id << " activated Little Friend" << std::endl;
        }
    }
}

}  // namespace server
