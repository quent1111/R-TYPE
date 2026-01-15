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
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, int client_id) {
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

        std::cout << "[PowerupHandler] Client " << client_id << " chose card "
                  << static_cast<int>(powerup_choice)
                  << " - PowerupId=" << static_cast<int>(chosen_card.id)
                  << " Level=" << static_cast<int>(chosen_card.level) << std::endl;

        powerups.add_or_upgrade(chosen_card.id);

        auto* def = powerup::PowerupRegistry::instance().get_powerup(chosen_card.id);
        if (def) {
            std::cout << "[Game] Client " << client_id << " received: " << def->name << " Level "
                      << static_cast<int>(powerups.get_level(chosen_card.id)) << std::endl;
        }

        if (def && def->category == powerup::PowerupCategory::Activable) {
            uint8_t level = powerups.get_level(chosen_card.id);
            bool assigned = powerups.assign_to_slot(chosen_card.id, level);
            if (assigned) {
                std::cout << "[Game] Assigned " << def->name << " to activable slot" << std::endl;
            }
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
    uint8_t slot_index) {
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

    if (slot_index >= 2) {
        return;
    }

    const auto* slot = powerups.get_slot(static_cast<int>(slot_index));
    if (!slot || !slot->has_powerup() || !slot->is_ready()) {
        return;
    }

    powerup::PowerupId id = slot->powerup_id.value();
    uint8_t level = slot->level;

    if (!powerups.activate_slot(slot_index)) {
        return;
    }

    auto* def = powerup::PowerupRegistry::instance().get_powerup(id);
    if (def) {
        std::cout << "[Game] Client " << client_id << " activated slot "
                  << static_cast<int>(slot_index) << " - " << def->name << " Level "
                  << static_cast<int>(level) << std::endl;
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
    } else if (id == powerup::PowerupId::LaserBeam) {
        if (def && level > 0 && level <= def->level_effects.size()) {
            const auto& effect = def->level_effects[level - 1];
            float duration = effect.duration;
            float dps = effect.value;

            auto& laser_opt = reg.get_component<laser_beam>(player);
            if (laser_opt.has_value()) {
                laser_opt->max_duration = duration;
                laser_opt->damage_per_second = dps;
                laser_opt->level = level;
                laser_opt->activate();
            } else {
                reg.emplace_component<laser_beam>(player, duration, dps, level);
                reg.get_component<laser_beam>(player)->activate();
            }
            std::cout << "[Game] Laser beam activated: " << duration << "s duration, " << dps
                      << " DPS" << std::endl;
        }
    }
}

void PowerupHandler::apply_passive_powerup(registry& reg, entity player, powerup::PowerupId id,
                                           uint8_t level) {
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
    } else if (id == powerup::PowerupId::MissileDrone) {
        auto& drone_opt = reg.get_component<missile_drone>(player);
        if (!drone_opt.has_value()) {
            reg.emplace_component<missile_drone>(player);
        }

        auto& md = reg.get_component<missile_drone>(player).value();

        auto* def = powerup::PowerupRegistry::instance().get_powerup(id);
        if (def && level > 0 && level <= def->level_effects.size()) {
            const auto& effect = def->level_effects[level - 1];
            md.missiles_per_volley = static_cast<int>(effect.value);
            md.num_drones = level;
        }

        md.activate(md.num_drones, md.missiles_per_volley);

        std::cout << "[Game] Missile Drone passive activated at level " << static_cast<int>(level)
                  << " with " << md.num_drones << " drone(s), " << md.missiles_per_volley
                  << " missiles per volley" << std::endl;
    }
}

void PowerupHandler::apply_stat_powerup(registry& reg, entity player, powerup::PowerupId id,
                                        uint8_t level) {
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
    } else if (id == powerup::PowerupId::Speed) {
        auto& control_opt = reg.get_component<controllable>(player);
        if (control_opt.has_value()) {
            float base_speed = 200.0f;
            control_opt->speed = base_speed * effect.value;
            std::cout << "[Game] Speed increased to " << control_opt->speed << std::endl;
        }
    } else if (id == powerup::PowerupId::Health) {
        auto& health_opt = reg.get_component<health>(player);
        if (health_opt.has_value()) {
            int bonus = static_cast<int>(effect.value);
            health_opt->maximum += bonus;
            health_opt->current += bonus;
            std::cout << "[Game] Max health increased by " << bonus << " to " << health_opt->maximum
                      << std::endl;
        }
    } else if (id == powerup::PowerupId::FireRate) {
        auto& weapon_opt = reg.get_component<weapon>(player);
        if (weapon_opt.has_value()) {
            float base_fire_rate = 5.0f;
            weapon_opt->fire_rate = base_fire_rate * effect.value;
            std::cout << "[Game] Fire rate increased by " << ((effect.value - 1.0f) * 100.0f)
                      << "% (new rate: " << weapon_opt->fire_rate << " shots/s)" << std::endl;
        }
    } else if (id == powerup::PowerupId::MultiShot) {
        auto& multishot_opt = reg.get_component<multishot>(player);
        if (!multishot_opt.has_value()) {
            reg.emplace_component<multishot>(player, static_cast<int>(effect.value));
        } else {
            multishot_opt->extra_projectiles = static_cast<int>(effect.value);
        }
        std::cout << "[Game] Multi-shot activated: " << static_cast<int>(effect.value)
                  << " extra projectiles" << std::endl;
    }
}

}  // namespace server
