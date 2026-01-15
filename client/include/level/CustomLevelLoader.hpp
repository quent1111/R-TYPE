#pragma once

#include "level/CustomLevelConfig.hpp"

#include <optional>
#include <string>

namespace level {

class CustomLevelLoader {
public:
    static std::optional<CustomLevelConfig> load(const std::string& level_id);
    static std::optional<CustomLevelConfig> load_from_file(const std::string& filepath);

private:
    static SpriteConfig parse_sprite(const void* json_obj);
    static ProjectileConfig parse_projectile(const void* json_obj);
    static EnemyDefinition parse_enemy(const void* json_obj);
    static BossDefinition parse_boss(const void* json_obj);
    static EnvironmentConfig parse_environment(const void* json_obj);
};

}  // namespace level
