#include "level/CustomLevelLoader.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace level {

using json = nlohmann::json;

SpriteConfig CustomLevelLoader::parse_sprite(const void* json_ptr) {
    const json& j = *static_cast<const json*>(json_ptr);
    SpriteConfig config;

    if (j.contains("texture_path"))
        config.texture_path = j["texture_path"].get<std::string>();
    if (j.contains("frame_width"))
        config.frame_width = j["frame_width"].get<int>();
    if (j.contains("frame_height"))
        config.frame_height = j["frame_height"].get<int>();
    if (j.contains("frame_count"))
        config.frame_count = j["frame_count"].get<int>();
    if (j.contains("frame_duration"))
        config.frame_duration = j["frame_duration"].get<float>();
    if (j.contains("scale_x"))
        config.scale_x = j["scale_x"].get<float>();
    if (j.contains("scale_y"))
        config.scale_y = j["scale_y"].get<float>();
    if (j.contains("mirror_x"))
        config.mirror_x = j["mirror_x"].get<bool>();
    if (j.contains("mirror_y"))
        config.mirror_y = j["mirror_y"].get<bool>();
    if (j.contains("rotation"))
        config.rotation = j["rotation"].get<float>();

    return config;
}

ProjectileConfig CustomLevelLoader::parse_projectile(const void* json_ptr) {
    const json& j = *static_cast<const json*>(json_ptr);
    ProjectileConfig config;

    if (j.contains("type"))
        config.type = j["type"].get<std::string>();
    if (j.contains("speed"))
        config.speed = j["speed"].get<float>();
    if (j.contains("damage"))
        config.damage = j["damage"].get<int>();
    if (j.contains("sprite")) {
        const json& sprite_json = j["sprite"];
        config.sprite = parse_sprite(&sprite_json);
    }

    return config;
}

EnemyDefinition CustomLevelLoader::parse_enemy(const void* json_ptr) {
    const json& j = *static_cast<const json*>(json_ptr);
    EnemyDefinition enemy;

    if (j.contains("id"))
        enemy.id = j["id"].get<std::string>();
    if (j.contains("name"))
        enemy.name = j["name"].get<std::string>();
    if (j.contains("health"))
        enemy.health = j["health"].get<int>();
    if (j.contains("speed"))
        enemy.speed = j["speed"].get<float>();
    if (j.contains("damage"))
        enemy.damage = j["damage"].get<int>();
    if (j.contains("score_value"))
        enemy.score_value = j["score_value"].get<int>();

    if (j.contains("sprite")) {
        const json& sprite_json = j["sprite"];
        enemy.sprite = parse_sprite(&sprite_json);
    }

    if (j.contains("attack") && j["attack"].contains("projectile")) {
        const json& proj_json = j["attack"]["projectile"];
        enemy.projectile = parse_projectile(&proj_json);
    }

    return enemy;
}

BossDefinition CustomLevelLoader::parse_boss(const void* json_ptr) {
    const json& j = *static_cast<const json*>(json_ptr);
    BossDefinition boss;

    if (j.contains("id"))
        boss.id = j["id"].get<std::string>();
    if (j.contains("name"))
        boss.name = j["name"].get<std::string>();
    if (j.contains("health"))
        boss.health = j["health"].get<int>();
    if (j.contains("damage"))
        boss.damage = j["damage"].get<int>();
    if (j.contains("score_value"))
        boss.score_value = j["score_value"].get<int>();

    if (j.contains("sprite")) {
        const json& sprite_json = j["sprite"];
        boss.sprite = parse_sprite(&sprite_json);
    }

    if (j.contains("attack") && j["attack"].contains("projectile")) {
        const json& proj_json = j["attack"]["projectile"];
        boss.projectile = parse_projectile(&proj_json);
    }

    return boss;
}

EnvironmentConfig CustomLevelLoader::parse_environment(const void* json_ptr) {
    const json& j = *static_cast<const json*>(json_ptr);
    EnvironmentConfig config;

    if (j.contains("background_texture"))
        config.background_texture = j["background_texture"].get<std::string>();
    if (j.contains("scroll_speed"))
        config.scroll_speed = j["scroll_speed"].get<float>();
    if (j.contains("scroll_infinite"))
        config.scroll_infinite = j["scroll_infinite"].get<bool>();
    if (j.contains("background_static"))
        config.background_static = j["background_static"].get<bool>();

    return config;
}

std::optional<CustomLevelConfig> CustomLevelLoader::load(const std::string& level_id) {
    std::string filepath = "levels/custom/" + level_id + ".json";
    if (std::filesystem::exists(filepath)) {
        return load_from_file(filepath);
    }

    std::filesystem::path custom_dir = "levels/custom";
    if (!std::filesystem::exists(custom_dir)) {
        std::cerr << "[CustomLevelLoader] Directory not found: " << custom_dir << std::endl;
        return std::nullopt;
    }

    for (const auto& entry : std::filesystem::directory_iterator(custom_dir)) {
        if (entry.path().extension() == ".json") {
            auto config = load_from_file(entry.path().string());
            if (config && config->id == level_id) {
                return config;
            }
        }
    }

    std::cerr << "[CustomLevelLoader] Level not found with ID: " << level_id << std::endl;
    return std::nullopt;
}

std::optional<CustomLevelConfig> CustomLevelLoader::load_from_file(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) {
        std::cerr << "[CustomLevelLoader] File not found: " << filepath << std::endl;
        return std::nullopt;
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[CustomLevelLoader] Cannot open file: " << filepath << std::endl;
        return std::nullopt;
    }

    try {
        json j;
        file >> j;

        CustomLevelConfig config;

        if (j.contains("metadata")) {
            const auto& meta = j["metadata"];
            if (meta.contains("id"))
                config.id = meta["id"].get<std::string>();
            if (meta.contains("name"))
                config.name = meta["name"].get<std::string>();
        }

        if (j.contains("environment")) {
            const json& env_json = j["environment"];
            config.environment = parse_environment(&env_json);
        }

        if (j.contains("enemy_definitions")) {
            for (auto& [key, value] : j["enemy_definitions"].items()) {
                bool is_boss = false;
                if (value.contains("behavior") && value["behavior"].contains("type")) {
                    is_boss = (value["behavior"]["type"].get<std::string>() == "boss");
                }

                if (is_boss) {
                    config.boss_definition = parse_boss(&value);
                } else {
                    config.enemy_definitions[key] = parse_enemy(&value);
                }
            }
        }

        std::cout << "[CustomLevelLoader] Loaded level: " << config.name << " ("
                  << config.enemy_definitions.size() << " enemies";
        if (config.boss_definition) {
            std::cout << ", 1 boss";
        }
        std::cout << ")" << std::endl;

        return config;

    } catch (const std::exception& e) {
        std::cerr << "[CustomLevelLoader] Parse error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

}  // namespace level
