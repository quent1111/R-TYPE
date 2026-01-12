#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace powerup {

enum class PowerupCategory : uint8_t {
    Activable = 0,
    Passive = 1,
    Stat = 2
};

enum class PowerupId : uint8_t {
    PowerCannon = 1,
    Shield = 2,
    LittleFriend = 10,
    Damage = 20,
    Speed = 21,
    Health = 22,
    FireRate = 23,
    MultiShot = 24,
    LaserBeam = 3,
    MissileDrone = 11
};

struct PowerupDefinition {
    PowerupId id;
    std::string name;
    std::string description;
    PowerupCategory category;
    uint8_t max_level;
    std::string asset_path;

    struct LevelEffect {
        float duration;
        float value;
        float cooldown;
        std::string description;
    };

    std::vector<LevelEffect> level_effects;
};

class PowerupRegistry {
public:
    static PowerupRegistry& instance() {
        static PowerupRegistry registry;
        return registry;
    }

    void initialize();

    const PowerupDefinition* get_powerup(PowerupId id) const;
    const PowerupDefinition* get_powerup(uint8_t id) const;

    std::vector<PowerupId> get_all_powerup_ids() const;
    std::vector<PowerupId> get_powerups_by_category(PowerupCategory category) const;

    PowerupId get_upgraded_version(PowerupId id, uint8_t current_level) const;

    PowerupRegistry(const PowerupRegistry&) = delete;
    PowerupRegistry& operator=(const PowerupRegistry&) = delete;

private:
    PowerupRegistry() = default;

    std::unordered_map<PowerupId, PowerupDefinition> powerups_;

    void register_powerup(const PowerupDefinition& def);
};

inline uint16_t make_powerup_key(PowerupId id, uint8_t level) {
    return static_cast<uint16_t>((static_cast<uint16_t>(id) << 8) | static_cast<uint16_t>(level));
}

inline PowerupId extract_powerup_id(uint16_t key) {
    return static_cast<PowerupId>(key >> 8);
}

inline uint8_t extract_powerup_level(uint16_t key) {
    return static_cast<uint8_t>(key & 0xFF);
}

}
