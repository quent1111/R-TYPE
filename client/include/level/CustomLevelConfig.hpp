#pragma once

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace level {

struct SpriteConfig {
    std::string texture_path;
    int frame_width = 32;
    int frame_height = 32;
    int frame_count = 1;
    float frame_duration = 0.1f;
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    bool mirror_x = false;
    bool mirror_y = false;
    float rotation = 0.0f;
};

struct ProjectileConfig {
    std::string type;
    SpriteConfig sprite;
    float speed = 300.0f;
    int damage = 10;
};

struct EnemyDefinition {
    std::string id;
    std::string name;
    SpriteConfig sprite;
    int health = 30;
    float speed = 100.0f;
    int damage = 10;
    int score_value = 100;
    std::optional<ProjectileConfig> projectile;
};

struct BossDefinition {
    std::string id;
    std::string name;
    SpriteConfig sprite;
    int health = 500;
    int damage = 50;
    int score_value = 1000;
    std::optional<ProjectileConfig> projectile;
};

struct EnvironmentConfig {
    std::string background_texture;
    float scroll_speed = 50.0f;
    bool scroll_infinite = true;
    bool background_static = false;
};

struct CustomLevelConfig {
    std::string id;
    std::string name;
    EnvironmentConfig environment;
    std::map<std::string, EnemyDefinition> enemy_definitions;
    std::optional<BossDefinition> boss_definition;

    bool is_loaded() const { return !id.empty(); }

    const EnemyDefinition* get_enemy_by_index(size_t index) const {
        if (enemy_definitions.empty())
            return nullptr;

        std::vector<std::string> keys;
        for (const auto& [key, _] : enemy_definitions) {
            keys.push_back(key);
        }
        std::sort(keys.begin(), keys.end());

        size_t actual_index = index % keys.size();
        auto it = enemy_definitions.find(keys[actual_index]);
        if (it != enemy_definitions.end()) {
            return &it->second;
        }
        return nullptr;
    }

    const std::vector<std::string> get_all_texture_paths() const {
        std::vector<std::string> paths;
        paths.push_back(environment.background_texture);
        for (const auto& [key, enemy] : enemy_definitions) {
            paths.push_back(enemy.sprite.texture_path);
            if (enemy.projectile) {
                paths.push_back(enemy.projectile->sprite.texture_path);
            }
        }
        if (boss_definition) {
            paths.push_back(boss_definition->sprite.texture_path);
            if (boss_definition->projectile) {
                paths.push_back(boss_definition->projectile->sprite.texture_path);
            }
        }
        return paths;
    }
};

}  // namespace level
