#pragma once

#include "PowerupRegistry.hpp"
#include <map>
#include <vector>

namespace powerup {

struct PlayerPowerups {
    std::map<PowerupId, uint8_t> owned_powerups;
    struct ActivePowerup {
        PowerupId id;
        uint8_t level;
        float time_remaining;
        bool is_active;
    };
    
    std::map<PowerupId, ActivePowerup> active_powerups;
    void add_or_upgrade(PowerupId id) {
        auto it = owned_powerups.find(id);
        if (it == owned_powerups.end()) {
            owned_powerups[id] = 1;
        } else {
            auto* def = PowerupRegistry::instance().get_powerup(id);
            if (def && it->second < def->max_level) {
                it->second++;
            }
        }
    }
    bool has_powerup(PowerupId id) const {
        return owned_powerups.find(id) != owned_powerups.end();
    }
    uint8_t get_level(PowerupId id) const {
        auto it = owned_powerups.find(id);
        return (it != owned_powerups.end()) ? it->second : 0;
    }
    void activate(PowerupId id) {
        uint8_t level = get_level(id);
        if (level == 0) return;
        auto* def = PowerupRegistry::instance().get_powerup(id);
        if (!def || def->category != PowerupCategory::Activable) return;
        if (level > def->level_effects.size()) return;
        const auto& effect = def->level_effects[level - 1];
        ActivePowerup active;
        active.id = id;
        active.level = level;
        active.time_remaining = effect.duration;
        active.is_active = true;
        active_powerups[id] = active;
    }
    void update(float dt) {
        for (auto it = active_powerups.begin(); it != active_powerups.end();) {
            if (it->second.is_active) {
                it->second.time_remaining -= dt;
                if (it->second.time_remaining <= 0.0f) {
                    it->second.is_active = false;
                    it->second.time_remaining = 0.0f;
                    it = active_powerups.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
    bool is_active(PowerupId id) const {
        auto it = active_powerups.find(id);
        return (it != active_powerups.end()) && it->second.is_active;
    }
    const ActivePowerup* get_active_info(PowerupId id) const {
        auto it = active_powerups.find(id);
        return (it != active_powerups.end()) ? &it->second : nullptr;
    }
    float get_damage_multiplier() const {
        uint8_t level = get_level(PowerupId::Damage);
        if (level == 0) return 1.0f;
        auto* def = PowerupRegistry::instance().get_powerup(PowerupId::Damage);
        if (!def || level > def->level_effects.size()) return 1.0f;
        return def->level_effects[level - 1].value;
    }
    float get_speed_multiplier() const {
        uint8_t level = get_level(PowerupId::Speed);
        if (level == 0) return 1.0f;
        auto* def = PowerupRegistry::instance().get_powerup(PowerupId::Speed);
        if (!def || level > def->level_effects.size()) return 1.0f;
        return def->level_effects[level - 1].value;
    }
    int get_max_health_bonus() const {
        uint8_t level = get_level(PowerupId::Health);
        if (level == 0) return 0;
        auto* def = PowerupRegistry::instance().get_powerup(PowerupId::Health);
        if (!def || level > def->level_effects.size()) return 0;
        return static_cast<int>(def->level_effects[level - 1].value);
    }
};

}  // namespace powerup
