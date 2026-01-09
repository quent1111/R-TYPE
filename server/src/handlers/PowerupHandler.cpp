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

std::vector<powerup::PowerupCard> PowerupHandler::generate_card_choices(
    registry& reg,
    const std::unordered_map<int, std::size_t>& client_entity_ids,
    int client_id) {
    
    auto player_opt = get_player_entity(reg, client_entity_ids, client_id);
    if (!player_opt.has_value()) {
        return {};
    }
    
    auto player = player_opt.value();
    
    auto& powerups_opt = reg.get_component<player_powerups_component>(player);
    if (!powerups_opt.has_value()) {
        reg.emplace_component<player_powerups_component>(player);
    }
    
    auto& powerups = reg.get_component<player_powerups_component>(player).value();
    
    auto cards = card_pool_.generate_card_choices(powerups, 3);
    
    player_card_choices_[client_id] = cards;
    
    return cards;
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

    auto& powerups_opt = reg.get_component<player_powerups_component>(player);
    if (!powerups_opt.has_value()) {
        reg.emplace_component<player_powerups_component>(player);
    }
    
    auto& powerups = reg.get_component<player_powerups_component>(player).value();
    
    auto it = player_card_choices_.find(client_id);
    if (it == player_card_choices_.end()) {
        std::cerr << "[PowerupHandler] No stored cards found for client " << client_id << std::endl;
        return;
    }
    
    const auto& cards = it->second;
    
    if (powerup_choice > 0 && powerup_choice <= cards.size()) {
        const auto& chosen_card = cards[powerup_choice - 1];
        
        std::cout << "[PowerupHandler] Client " << client_id 
                  << " chose card " << static_cast<int>(powerup_choice) 
                  << " - PowerupId=" << static_cast<int>(chosen_card.id) 
                  << " Level=" << static_cast<int>(chosen_card.level) << std::endl;
        
        powerups.add_or_upgrade(chosen_card.id);
        
        auto* def = powerup::PowerupRegistry::instance().get_powerup(chosen_card.id);
        if (def) {
            std::cout << "[Game] Client " << client_id << " received: " 
                      << def->name << " Level " << static_cast<int>(powerups.get_level(chosen_card.id))
                      << std::endl;
        }
        
        if (def && def->category == powerup::PowerupCategory::Passive) {
            apply_passive_powerup(reg, player, chosen_card.id, powerups.get_level(chosen_card.id));
        }
        
        if (def && def->category == powerup::PowerupCategory::Stat) {
            apply_stat_powerup(reg, player, chosen_card.id, powerups.get_level(chosen_card.id));
        }
    }

    players_who_chose_powerup.insert(client_id);
    
    player_card_choices_.erase(client_id);

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
    
    auto& powerups_opt = reg.get_component<player_powerups_component>(player);
    if (!powerups_opt.has_value()) {
        return;
    }
    
    auto& powerups = powerups_opt.value();
    
    powerup::PowerupId id;
    if (powerup_type == 1) {
        id = powerup::PowerupId::PowerCannon;
    } else if (powerup_type == 2) {
        id = powerup::PowerupId::Shield;
    } else {
        return;
    }
    
    if (!powerups.has_powerup(id)) {
        return;
    }
    
    uint8_t level = powerups.get_level(id);
    powerups.activate(id);
    
    auto* def = powerup::PowerupRegistry::instance().get_powerup(id);
    if (def) {
        std::cout << "[Game] Client " << client_id << " activated " << def->name 
                  << " Level " << static_cast<int>(level) << std::endl;
    }
    
    if (id == powerup::PowerupId::PowerCannon) {
        if (def && level > 0 && level <= def->level_effects.size()) {
            const auto& effect = def->level_effects[level - 1];
            float duration = effect.duration;
            int damage = static_cast<int>(effect.value);
            
            auto& cannon_opt = reg.get_component<power_cannon>(player);
            if (cannon_opt.has_value()) {
                cannon_opt->activate(duration, damage);
            } else {
                reg.emplace_component<power_cannon>(player);
                reg.get_component<power_cannon>(player)->activate(duration, damage);
            }
        }
    } else if (id == powerup::PowerupId::Shield) {
        if (def && level > 0 && level <= def->level_effects.size()) {
            const auto& effect = def->level_effects[level - 1];
            float duration = effect.duration;
            float radius = effect.value;
            
            auto& shield_opt = reg.get_component<shield>(player);
            if (shield_opt.has_value()) {
                shield_opt->activate(duration, radius);
            } else {
                reg.emplace_component<shield>(player);
                reg.get_component<shield>(player)->activate(duration, radius);
            }
        }
    }
}

void PowerupHandler::apply_passive_powerup(registry& reg, entity player, 
                                           powerup::PowerupId id, uint8_t level) {
    if (id == powerup::PowerupId::LittleFriend) {
        auto& friend_opt = reg.get_component<little_friend>(player);
        if (!friend_opt.has_value()) {
            reg.emplace_component<little_friend>(player);
        }
        
        auto& lf = reg.get_component<little_friend>(player).value();
        
        auto* def = powerup::PowerupRegistry::instance().get_powerup(id);
        if (def && level > 0 && level <= def->level_effects.size()) {
            const auto& effect = def->level_effects[level - 1];
            lf.damage = static_cast<int>(effect.value);
            lf.fire_rate = effect.cooldown;

            lf.num_drones = (level >= 3) ? 2 : 1;
        }

        lf.duration = 999999.0f;
        lf.activate();

        std::cout << "[Game] Little Friend passive activated at level " << static_cast<int>(level)
                  << " with " << lf.num_drones << " drone(s)" << std::endl;
    }
}

void PowerupHandler::apply_stat_powerup(registry& reg, entity player,
                                        powerup::PowerupId id, uint8_t level) {
    auto* def = powerup::PowerupRegistry::instance().get_powerup(id);
    if (!def || level == 0 || level > def->level_effects.size()) {
        return;
    }
    
    const auto& effect = def->level_effects[level - 1];
    
    if (id == powerup::PowerupId::Damage) {
        auto& weapon_opt = reg.get_component<weapon>(player);
        if (weapon_opt.has_value()) {
            float base_damage = 10.0f;
            weapon_opt->damage = static_cast<int>(base_damage * effect.value);
            std::cout << "[Game] Damage increased to " << weapon_opt->damage << std::endl;
        }
    }
    else if (id == powerup::PowerupId::Speed) {
        auto& control_opt = reg.get_component<controllable>(player);
        if (control_opt.has_value()) {
            float base_speed = 200.0f;
            control_opt->speed = base_speed * effect.value;
            std::cout << "[Game] Speed increased to " << control_opt->speed << std::endl;
        }
    }
    else if (id == powerup::PowerupId::Health) {
        auto& health_opt = reg.get_component<health>(player);
        if (health_opt.has_value()) {
            int bonus = static_cast<int>(effect.value);
            health_opt->maximum += bonus;
            health_opt->current += bonus;
            std::cout << "[Game] Max health increased by " << bonus << " to " << health_opt->maximum << std::endl;
        }
    }
}

}  // namespace server
