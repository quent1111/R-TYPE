#include "handlers/WeaponHandler.hpp"

namespace server {

std::optional<entity> WeaponHandler::get_player_entity(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id) {
    auto it = client_entity_ids.find(client_id);
    if (it != client_entity_ids.end()) {
        return reg.entity_from_index(it->second);
    }
    return std::nullopt;
}

bool WeaponHandler::handle_weapon_upgrade_choice(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id,
    uint8_t upgrade_choice) {
    auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
    if (!player_opt.has_value()) {
        std::cerr << "[Game] Cannot apply upgrade: player not found for client " << client_id
                  << std::endl;
        return false;
    }
    auto player = player_opt.value();
    auto& wpn_opt = reg.get_component<weapon>(player);
    if (!wpn_opt.has_value()) {
        std::cerr << "[Game] Cannot apply upgrade: player has no weapon component" << std::endl;
        return false;
    }
    WeaponUpgradeType upgrade_type = WeaponUpgradeType::None;
    std::string upgrade_name = "None";
    if (upgrade_choice == 1) {
        upgrade_type = WeaponUpgradeType::PowerShot;
        upgrade_name = "Power Shot";
    } else if (upgrade_choice == 2) {
        upgrade_type = WeaponUpgradeType::TripleShot;
        upgrade_name = "Triple Shot";
    }
    wpn_opt->apply_upgrade(upgrade_type);
    std::cout << "[Game] Client " << client_id << " chose upgrade: " << upgrade_name << std::endl;
    int players_ready = 0;
    int total_players = 0;
    for (const auto& [cid, entity_id] : client_entity_ids) {
        total_players++;
        auto ent = reg.entity_from_index(entity_id);
        auto wpn = reg.get_component<weapon>(ent);
        if (wpn.has_value() && wpn->upgrade_type != WeaponUpgradeType::None) {
            players_ready++;
        }
    }
    return (players_ready == total_players && total_players > 0);
}

}  // namespace server
