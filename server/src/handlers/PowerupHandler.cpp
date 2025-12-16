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
    std::unordered_set<int>& players_who_chose_powerup, int client_id, uint8_t powerup_choice) {
    auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
    if (!player_opt.has_value()) {
        std::cerr << "[Game] Cannot apply power-up: player not found for client " << client_id
                  << std::endl;
        return;
    }

    auto player = player_opt.value();
    if (powerup_choice == 1) {
        reg.emplace_component<power_cannon>(player);
        std::cout << "[Game] Client " << client_id << " chose Power Cannon" << std::endl;
    } else if (powerup_choice == 2) {
        reg.emplace_component<shield>(player);
        std::cout << "[Game] Client " << client_id << " chose Shield" << std::endl;
    }

    players_who_chose_powerup.insert(client_id);

    int alive_players = count_alive_players(reg, client_entity_ids);

    std::cout << "[Game] Powerup choices: " << players_who_chose_powerup.size() << "/"
              << alive_players << " players" << std::endl;
}

void PowerupHandler::handle_powerup_activate(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id) {
    auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
    if (!player_opt.has_value()) {
        return;
    }
    auto player = player_opt.value();
    auto& cannon_opt = reg.get_component<power_cannon>(player);
    if (cannon_opt.has_value()) {
        if (!cannon_opt->is_active()) {
            cannon_opt->activate();
        }
        return;
    }
    auto& shield_opt = reg.get_component<shield>(player);
    if (shield_opt.has_value()) {
        if (!shield_opt->is_active()) {
            shield_opt->activate();
        }
        return;
    }
}

}  // namespace server
