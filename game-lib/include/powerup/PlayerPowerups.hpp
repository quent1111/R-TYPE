#pragma once

#include "PowerupRegistry.hpp"
#include <map>
#include <vector>
#include <optional>

namespace powerup {

struct PlayerPowerups {
    std::map<PowerupId, uint8_t> owned_powerups;
    
    struct ActivableSlot {
        std::optional<PowerupId> powerup_id;
        uint8_t level = 0;
        float time_remaining = 0.0f;
        float cooldown_remaining = 0.0f;
        bool is_active = false;
        
        bool has_powerup() const { return powerup_id.has_value(); }
        bool is_ready() const { return has_powerup() && cooldown_remaining <= 0.0f && !is_active; }
        bool is_on_cooldown() const { return cooldown_remaining > 0.0f; }
    };
    
    ActivableSlot activable_slots[2];
    
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
    
    bool assign_to_slot(PowerupId id, uint8_t level) {
        auto* def = PowerupRegistry::instance().get_powerup(id);
        if (!def || def->category != PowerupCategory::Activable) {
            return false;
        }
        
        for (int i = 0; i < 2; ++i) {
            if (activable_slots[i].powerup_id == id) {
                activable_slots[i].level = level;
                return true;
            }
        }
        
        for (int i = 0; i < 2; ++i) {
            if (!activable_slots[i].has_powerup()) {
                activable_slots[i].powerup_id = id;
                activable_slots[i].level = level;
                activable_slots[i].time_remaining = 0.0f;
                activable_slots[i].cooldown_remaining = 0.0f;
                activable_slots[i].is_active = false;
                return true;
            }
        }
        
        return false;
    }
    
    bool has_powerup(PowerupId id) const {
        return owned_powerups.find(id) != owned_powerups.end();
    }
    
    uint8_t get_level(PowerupId id) const {
        auto it = owned_powerups.find(id);
        return (it != owned_powerups.end()) ? it->second : 0;
    }
    
    int get_slot_index(PowerupId id) const {
        for (int i = 0; i < 2; ++i) {
            if (activable_slots[i].powerup_id == id) {
                return i;
            }
        }
        return -1;
    }
    
    bool activate_slot(int slot_index) {
        if (slot_index < 0 || slot_index >= 2) return false;
        
        auto& slot = activable_slots[slot_index];
        if (!slot.is_ready()) return false;
        
        auto* def = PowerupRegistry::instance().get_powerup(slot.powerup_id.value());
        if (!def || slot.level == 0 || slot.level > def->level_effects.size()) {
            return false;
        }
        
        const auto& effect = def->level_effects[slot.level - 1];
        slot.time_remaining = effect.duration;
        slot.is_active = true;
        
        return true;
    }
    
    void update(float dt) {
        for (int i = 0; i < 2; ++i) {
            auto& slot = activable_slots[i];
            if (!slot.has_powerup()) continue;
            
            if (slot.is_active) {
                slot.time_remaining -= dt;
                if (slot.time_remaining <= 0.0f) {
                    slot.is_active = false;
                    slot.time_remaining = 0.0f;
                    slot.cooldown_remaining = 25.0f;
                }
            } else if (slot.cooldown_remaining > 0.0f) {
                slot.cooldown_remaining -= dt;
                if (slot.cooldown_remaining < 0.0f) {
                    slot.cooldown_remaining = 0.0f;
                }
            }
        }
    }
    
    bool is_slot_active(int slot_index) const {
        if (slot_index < 0 || slot_index >= 2) return false;
        return activable_slots[slot_index].is_active;
    }
    
    const ActivableSlot* get_slot(int index) const {
        if (index < 0 || index >= 2) return nullptr;
        return &activable_slots[index];
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
