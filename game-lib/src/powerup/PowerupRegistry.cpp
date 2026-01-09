#include "powerup/PowerupRegistry.hpp"
#include <iostream>

namespace powerup {

void PowerupRegistry::initialize() {
    powerups_.clear();
    
    {
        PowerupDefinition def;
        def.id = PowerupId::PowerCannon;
        def.name = "Power Cannon";
        def.description = "Powerful charged cannon";
        def.category = PowerupCategory::Activable;
        def.max_level = 3;
        def.asset_path = "assets/canon_powerup.png";
        
        def.level_effects.push_back({
            10.0f,
            50.0f,
            0.3f,
            "Basic power cannon"
        });
        
        def.level_effects.push_back({
            12.0f,
            75.0f,
            0.25f,
            "Improved power cannon with faster fire rate"
        });

        def.level_effects.push_back({
            15.0f,
            100.0f,
            0.2f,
            "Ultimate power cannon with devastating damage"
        });

        register_powerup(def);
    }

    {
        PowerupDefinition def;
        def.id = PowerupId::Shield;
        def.name = "Energy Shield";
        def.description = "Protective energy barrier";
        def.category = PowerupCategory::Activable;
        def.max_level = 3;
        def.asset_path = "assets/shield_powerup.png";

        def.level_effects.push_back({
            10.0f,
            80.0f,
            0.0f,
            "Basic energy shield"
        });
        
        def.level_effects.push_back({
            15.0f,
            100.0f,
            0.0f,
            "Enhanced shield with larger radius"
        });
        
        def.level_effects.push_back({
            20.0f,
            120.0f,
            0.0f,
            "Ultimate shield with massive protection area"
        });
        
        register_powerup(def);
    }
    
    {
        PowerupDefinition def;
        def.id = PowerupId::LittleFriend;
        def.name = "Support Drone";
        def.description = "Permanent support ship that fights alongside you";
        def.category = PowerupCategory::Passive;
        def.max_level = 3;
        def.asset_path = "assets/support_powerup.png";
        
        def.level_effects.push_back({
            0.0f,
            15.0f,
            0.7f,
            "Basic support drone"
        });
        
        def.level_effects.push_back({
            0.0f,
            25.0f,
            0.6f,
            "Enhanced drone with improved firepower"
        });
        
        def.level_effects.push_back({
            0.0f,
            40.0f,
            0.7f,
            "2 elite drones with rapid fire capabilities"
        });
        
        register_powerup(def);
    }
    
    {
        PowerupDefinition def;
        def.id = PowerupId::Damage;
        def.name = "Attack Power";
        def.description = "Permanently increases weapon damage";
        def.category = PowerupCategory::Stat;
        def.max_level = 3;
        def.asset_path = "assets/attack_powerup.png";
        
        def.level_effects.push_back({
            0.0f,
            1.2f,
            0.0f,
            "+20% Weapon Damage"
        });
        
        def.level_effects.push_back({
            0.0f,
            1.5f,
            0.0f,
            "+50% Weapon Damage"
        });
        
        def.level_effects.push_back({
            0.0f,
            2.0f,
            0.0f,
            "+100% Weapon Damage (Double Damage!)"
        });
        
        register_powerup(def);
    }
    
    {
        PowerupDefinition def;
        def.id = PowerupId::Speed;
        def.name = "Speed Boost";
        def.description = "Permanently increases movement speed";
        def.category = PowerupCategory::Stat;
        def.max_level = 3;
        def.asset_path = "assets/speed_powerup.png";
        
        def.level_effects.push_back({
            0.0f,
            1.2f,
            0.0f,
            "+20% Movement Speed"
        });
        
        def.level_effects.push_back({
            0.0f,
            1.4f,
            0.0f,
            "+40% Movement Speed"
        });
        
        def.level_effects.push_back({
            0.0f,
            1.7f,
            0.0f,
            "+70% Movement Speed (Lightning Fast!)"
        });
        
        register_powerup(def);
    }
    
    {
        PowerupDefinition def;
        def.id = PowerupId::Health;
        def.name = "Max Health";
        def.description = "Permanently increases maximum health";
        def.category = PowerupCategory::Stat;
        def.max_level = 3;
        def.asset_path = "assets/health_powerup.png";
        
        def.level_effects.push_back({
            0.0f,
            30.0f,
            0.0f,
            "+30 Maximum Health"
        });
        
        def.level_effects.push_back({
            0.0f,
            50.0f,
            0.0f,
            "+50 Maximum Health"
        });
        
        def.level_effects.push_back({
            0.0f,
            80.0f,
            0.0f,
            "+80 Maximum Health (Tank!)"
        });
        
        register_powerup(def);
    }
    
    std::cout << "[PowerupRegistry] Initialized with " << powerups_.size() << " power-ups" << std::endl;
}

void PowerupRegistry::register_powerup(const PowerupDefinition& def) {
    powerups_[def.id] = def;
}

const PowerupDefinition* PowerupRegistry::get_powerup(PowerupId id) const {
    auto it = powerups_.find(id);
    if (it != powerups_.end()) {
        return &it->second;
    }
    return nullptr;
}

const PowerupDefinition* PowerupRegistry::get_powerup(uint8_t id) const {
    return get_powerup(static_cast<PowerupId>(id));
}

std::vector<PowerupId> PowerupRegistry::get_all_powerup_ids() const {
    std::vector<PowerupId> ids;
    ids.reserve(powerups_.size());
    for (const auto& [id, def] : powerups_) {
        ids.push_back(id);
    }
    return ids;
}

std::vector<PowerupId> PowerupRegistry::get_powerups_by_category(PowerupCategory category) const {
    std::vector<PowerupId> ids;
    for (const auto& [id, def] : powerups_) {
        if (def.category == category) {
            ids.push_back(id);
        }
    }
    return ids;
}

PowerupId PowerupRegistry::get_upgraded_version(PowerupId id, uint8_t current_level) const {
    auto* def = get_powerup(id);
    if (def && current_level < def->max_level) {
        return id;
    }
    return id;
}

}
