#include "level/LevelConfigParser.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace rtype::level {

using json = nlohmann::json;

template<typename T>
T getOrDefault(const json& j, const std::string& key, const T& default_value) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return default_value;
}

template<typename T>
std::optional<T> getOptional(const json& j, const std::string& key) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return std::nullopt;
}

SpriteConfig parseSpriteConfig(const json& j) {
    SpriteConfig config;
    config.texture_path = getOrDefault<std::string>(j, "texture_path", "");
    config.frame_width = getOrDefault(j, "frame_width", 32);
    config.frame_height = getOrDefault(j, "frame_height", 32);
    config.frame_count = getOrDefault(j, "frame_count", 1);
    config.frame_duration = getOrDefault(j, "frame_duration", 0.1f);
    config.scale_x = getOrDefault(j, "scale_x", 1.0f);
    config.scale_y = getOrDefault(j, "scale_y", 1.0f);
    config.mirror_x = getOrDefault(j, "mirror_x", false);
    config.mirror_y = getOrDefault(j, "mirror_y", false);
    config.rotation = getOrDefault(j, "rotation", 0.0f);
    return config;
}

ProjectileConfig parseProjectileConfig(const json& j) {
    ProjectileConfig config;
    config.type = getOrDefault<std::string>(j, "type", "basic");
    if (j.contains("sprite")) {
        config.sprite = parseSpriteConfig(j["sprite"]);
    }
    config.speed = getOrDefault(j, "speed", 400.0f);
    config.damage = getOrDefault(j, "damage", 10);
    config.fire_rate = getOrDefault(j, "fire_rate", 1.0f);
    config.homing = getOrDefault(j, "homing", false);
    config.homing_strength = getOrDefault(j, "homing_strength", 0.0f);
    return config;
}

MovementPatternConfig parseMovementPatternConfig(const json& j) {
    MovementPatternConfig config;
    config.type = getOrDefault<std::string>(j, "type", "linear");
    config.amplitude = getOrDefault(j, "amplitude", 0.0f);
    config.frequency = getOrDefault(j, "frequency", 0.0f);
    config.phase = getOrDefault(j, "phase", 0.0f);
    
    if (j.contains("waypoints") && j["waypoints"].is_array()) {
        for (const auto& wp : j["waypoints"]) {
            if (wp.is_array() && wp.size() >= 2) {
                config.waypoints.emplace_back(wp[0].get<float>(), wp[1].get<float>());
            }
        }
    }
    return config;
}

BehaviorConfig parseBehaviorConfig(const json& j) {
    BehaviorConfig config;
    config.type = getOrDefault<std::string>(j, "type", "straight");
    if (j.contains("movement")) {
        config.movement = parseMovementPatternConfig(j["movement"]);
    }
    config.tracks_player = getOrDefault(j, "tracks_player", false);
    config.tracking_speed = getOrDefault(j, "tracking_speed", 0.0f);
    config.aggro_range = getOrDefault(j, "aggro_range", 0.0f);
    return config;
}

AttackPatternConfig parseAttackPatternConfig(const json& j) {
    AttackPatternConfig config;
    config.type = getOrDefault<std::string>(j, "type", "none");
    config.cooldown = getOrDefault(j, "cooldown", 2.0f);
    config.burst_count = getOrDefault(j, "burst_count", 1);
    config.burst_delay = getOrDefault(j, "burst_delay", 0.1f);
    config.spread_angle = getOrDefault(j, "spread_angle", 30.0f);
    config.projectile_count = getOrDefault(j, "projectile_count", 1);
    config.aim_at_player = getOrDefault(j, "aim_at_player", false);
    if (j.contains("projectile")) {
        config.projectile = parseProjectileConfig(j["projectile"]);
    }
    return config;
}

EnemyConfig parseEnemyConfig(const json& j) {
    EnemyConfig config;
    config.id = getOrDefault<std::string>(j, "id", "");
    config.name = getOrDefault<std::string>(j, "name", "");
    if (j.contains("sprite")) {
        config.sprite = parseSpriteConfig(j["sprite"]);
    }
    config.health = getOrDefault(j, "health", 100);
    config.speed = getOrDefault(j, "speed", 100.0f);
    config.damage = getOrDefault(j, "damage", 10);
    config.score_value = getOrDefault(j, "score_value", 100);
    if (j.contains("behavior")) {
        config.behavior = parseBehaviorConfig(j["behavior"]);
    }
    if (j.contains("attack")) {
        config.attack = parseAttackPatternConfig(j["attack"]);
    }
    config.death_sound = getOptional<std::string>(j, "death_sound");
    if (j.contains("death_animation")) {
        config.death_animation = parseSpriteConfig(j["death_animation"]);
    }
    return config;
}

SpawnPointConfig parseSpawnPointConfig(const json& j) {
    SpawnPointConfig config;
    config.x = getOrDefault(j, "x", 0.0f);
    config.y = getOrDefault(j, "y", 0.0f);
    config.position_type = getOrDefault<std::string>(j, "position_type", "absolute");
    config.offset_x = getOrDefault(j, "offset_x", 0.0f);
    config.offset_y = getOrDefault(j, "offset_y", 0.0f);
    return config;
}

EnemySpawnConfig parseEnemySpawnConfig(const json& j) {
    EnemySpawnConfig config;
    config.enemy_id = getOrDefault<std::string>(j, "enemy_id", "");
    config.count = getOrDefault(j, "count", 1);
    config.spawn_delay = getOrDefault(j, "spawn_delay", 0.5f);
    if (j.contains("spawn_point")) {
        config.spawn_point = parseSpawnPointConfig(j["spawn_point"]);
    }
    config.formation = getOptional<std::string>(j, "formation");
    return config;
}

WaveConfig parseWaveConfig(const json& j) {
    WaveConfig config;
    config.wave_number = getOrDefault(j, "wave_number", 0);
    config.name = getOrDefault<std::string>(j, "name", "");
    config.wave_delay = getOrDefault(j, "wave_delay", 2.0f);
    config.is_boss_wave = getOrDefault(j, "is_boss_wave", false);
    config.trigger_condition = getOptional<std::string>(j, "trigger_condition");
    config.music_override = getOptional<std::string>(j, "music_override");
    
    if (j.contains("enemies") && j["enemies"].is_array()) {
        for (const auto& enemy : j["enemies"]) {
            config.enemies.push_back(parseEnemySpawnConfig(enemy));
        }
    }
    return config;
}

PowerupSpawnConfig parsePowerupSpawnConfig(const json& j) {
    PowerupSpawnConfig config;
    config.powerup_type = getOrDefault<std::string>(j, "powerup_type", "");
    config.spawn_chance = getOrDefault(j, "spawn_chance", 0.1f);
    config.spawn_on_wave = getOptional<int>(j, "spawn_on_wave");
    config.spawn_condition = getOptional<std::string>(j, "spawn_condition");
    return config;
}

EnvironmentConfig parseEnvironmentConfig(const json& j) {
    EnvironmentConfig config;
    config.background_texture = getOrDefault<std::string>(j, "background_texture", "");
    config.parallax_layer_1 = getOptional<std::string>(j, "parallax_layer_1");
    config.parallax_layer_2 = getOptional<std::string>(j, "parallax_layer_2");
    config.scroll_speed = getOrDefault(j, "scroll_speed", 50.0f);
    config.scroll_infinite = getOrDefault(j, "scroll_infinite", true);
    config.background_static = getOrDefault(j, "background_static", false);
    config.music = getOptional<std::string>(j, "music");
    config.ambient_sound = getOptional<std::string>(j, "ambient_sound");
    return config;
}

LevelMetadata parseLevelMetadata(const json& j) {
    LevelMetadata metadata;
    std::cout << "[Parser] parseLevelMetadata called" << std::endl;
    std::cout << "[Parser] JSON has 'id': " << j.contains("id") << std::endl;
    if (j.contains("id")) {
        std::cout << "[Parser] id value: " << j["id"].dump() << std::endl;
    }
    metadata.id = getOrDefault<std::string>(j, "id", "");
    std::cout << "[Parser] Parsed ID: '" << metadata.id << "'" << std::endl;
    metadata.name = getOrDefault<std::string>(j, "name", "");
    metadata.author = getOrDefault<std::string>(j, "author", "");
    metadata.version = getOrDefault<std::string>(j, "version", "1.0.0");
    metadata.description = getOrDefault<std::string>(j, "description", "");
    metadata.difficulty = getOrDefault(j, "difficulty", 1);
    metadata.preview_image = getOptional<std::string>(j, "preview_image");
    return metadata;
}

bool LevelConfigParser::validateConfig(const LevelConfig& config, std::vector<std::string>& warnings) {
    bool valid = true;
    
    if (config.metadata.id.empty()) {
        warnings.push_back("Level ID is empty");
        valid = false;
    }
    
    if (config.metadata.name.empty()) {
        warnings.push_back("Level name is empty");
    }
    
    if (config.waves.empty()) {
        warnings.push_back("No waves defined in level");
        valid = false;
    }
    
    for (const auto& wave : config.waves) {
        for (const auto& enemy_spawn : wave.enemies) {
            if (!enemy_spawn.enemy_id.empty() && 
                config.enemy_definitions.find(enemy_spawn.enemy_id) == config.enemy_definitions.end()) {
                warnings.push_back("Wave " + std::to_string(wave.wave_number) + 
                    " references undefined enemy: " + enemy_spawn.enemy_id);
            }
        }
    }
    
    return valid;
}

bool LevelConfigParser::isSuccess(const ParseReturn& result) {
    return std::holds_alternative<ParseResult>(result);
}

ParseResult& LevelConfigParser::getResult(ParseReturn& result) {
    return std::get<ParseResult>(result);
}

const ParseResult& LevelConfigParser::getResult(const ParseReturn& result) {
    return std::get<ParseResult>(result);
}

std::pair<ParseError, std::string>& LevelConfigParser::getError(ParseReturn& result) {
    return std::get<std::pair<ParseError, std::string>>(result);
}

ParseReturn LevelConfigParser::parse(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return std::make_pair(ParseError::FileNotFound, "File not found: " + path.string());
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::make_pair(ParseError::FileNotFound, "Cannot open file: " + path.string());
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    return parseFromString(content);
}

ParseReturn LevelConfigParser::parseFromString(const std::string& json_content) {
    std::cout << "[Parser] parseFromString called, content size: " << json_content.size() << std::endl;
    json j;
    try {
        j = json::parse(json_content);
        std::cout << "[Parser] JSON parsed successfully" << std::endl;
    } catch (const json::parse_error& e) {
        return std::make_pair(ParseError::InvalidJson, "JSON parse error: " + std::string(e.what()));
    }
    
    ParseResult result;
    
    try {
        std::cout << "[Parser] JSON contains 'metadata': " << j.contains("metadata") << std::endl;
        if (j.contains("metadata")) {
            std::cout << "[Parser] metadata value: " << j["metadata"].dump() << std::endl;
            result.config.metadata = parseLevelMetadata(j["metadata"]);
            std::cout << "[Parser] After parse, ID is: '" << result.config.metadata.id << "'" << std::endl;
        } else {
            std::cout << "[Parser] WARNING: No metadata found in JSON" << std::endl;
        }
        
        if (j.contains("environment")) {
            result.config.environment = parseEnvironmentConfig(j["environment"]);
        }
        
        if (j.contains("enemy_definitions") && j["enemy_definitions"].is_object()) {
            for (const auto& [key, value] : j["enemy_definitions"].items()) {
                auto enemy = parseEnemyConfig(value);
                if (enemy.id.empty()) {
                    enemy.id = key;
                }
                result.config.enemy_definitions[key] = enemy;
            }
        }
        
        if (j.contains("waves") && j["waves"].is_array()) {
            int wave_num = 1;
            for (const auto& wave : j["waves"]) {
                auto wave_config = parseWaveConfig(wave);
                if (wave_config.wave_number == 0) {
                    wave_config.wave_number = wave_num++;
                }
                result.config.waves.push_back(wave_config);
            }
        }
        
        if (j.contains("powerups") && j["powerups"].is_array()) {
            for (const auto& powerup : j["powerups"]) {
                result.config.powerups.push_back(parsePowerupSpawnConfig(powerup));
            }
        }
        
        result.config.max_players = getOptional<int>(j, "max_players");
        result.config.time_limit = getOptional<float>(j, "time_limit");
        result.config.lives = getOptional<int>(j, "lives");
        
    } catch (const std::exception& e) {
        return std::make_pair(ParseError::InvalidValue, "Error parsing config: " + std::string(e.what()));
    }
    
    validateConfig(result.config, result.warnings);
    
    return result;
}

std::string LevelConfigParser::serialize(const LevelConfig& config) {
    json j;
    
    j["metadata"] = {
        {"id", config.metadata.id},
        {"name", config.metadata.name},
        {"author", config.metadata.author},
        {"version", config.metadata.version},
        {"description", config.metadata.description},
        {"difficulty", config.metadata.difficulty}
    };
    if (config.metadata.preview_image) {
        j["metadata"]["preview_image"] = *config.metadata.preview_image;
    }
    
    j["environment"] = {
        {"background_texture", config.environment.background_texture},
        {"scroll_speed", config.environment.scroll_speed}
    };
    if (config.environment.parallax_layer_1) {
        j["environment"]["parallax_layer_1"] = *config.environment.parallax_layer_1;
    }
    if (config.environment.parallax_layer_2) {
        j["environment"]["parallax_layer_2"] = *config.environment.parallax_layer_2;
    }
    if (config.environment.music) {
        j["environment"]["music"] = *config.environment.music;
    }
    if (config.environment.ambient_sound) {
        j["environment"]["ambient_sound"] = *config.environment.ambient_sound;
    }
    
    j["enemy_definitions"] = json::object();
    for (const auto& [id, enemy] : config.enemy_definitions) {
        j["enemy_definitions"][id] = {
            {"id", enemy.id},
            {"name", enemy.name},
            {"sprite", {
                {"texture_path", enemy.sprite.texture_path},
                {"frame_width", enemy.sprite.frame_width},
                {"frame_height", enemy.sprite.frame_height},
                {"frame_count", enemy.sprite.frame_count},
                {"frame_duration", enemy.sprite.frame_duration},
                {"scale_x", enemy.sprite.scale_x},
                {"scale_y", enemy.sprite.scale_y}
            }},
            {"health", enemy.health},
            {"speed", enemy.speed},
            {"damage", enemy.damage},
            {"score_value", enemy.score_value},
            {"behavior", {
                {"type", enemy.behavior.type},
                {"tracks_player", enemy.behavior.tracks_player},
                {"tracking_speed", enemy.behavior.tracking_speed},
                {"aggro_range", enemy.behavior.aggro_range},
                {"movement", {
                    {"type", enemy.behavior.movement.type},
                    {"amplitude", enemy.behavior.movement.amplitude},
                    {"frequency", enemy.behavior.movement.frequency},
                    {"phase", enemy.behavior.movement.phase}
                }}
            }},
            {"attack", {
                {"type", enemy.attack.type},
                {"cooldown", enemy.attack.cooldown},
                {"burst_count", enemy.attack.burst_count},
                {"burst_delay", enemy.attack.burst_delay},
                {"spread_angle", enemy.attack.spread_angle},
                {"projectile_count", enemy.attack.projectile_count}
            }}
        };
    }
    
    j["waves"] = json::array();
    for (const auto& wave : config.waves) {
        json wave_json = {
            {"wave_number", wave.wave_number},
            {"name", wave.name},
            {"wave_delay", wave.wave_delay},
            {"is_boss_wave", wave.is_boss_wave}
        };
        
        wave_json["enemies"] = json::array();
        for (const auto& enemy : wave.enemies) {
            json enemy_json = {
                {"enemy_id", enemy.enemy_id},
                {"count", enemy.count},
                {"spawn_delay", enemy.spawn_delay},
                {"spawn_point", {
                    {"x", enemy.spawn_point.x},
                    {"y", enemy.spawn_point.y},
                    {"position_type", enemy.spawn_point.position_type}
                }}
            };
            if (enemy.formation) {
                enemy_json["formation"] = *enemy.formation;
            }
            wave_json["enemies"].push_back(enemy_json);
        }
        
        j["waves"].push_back(wave_json);
    }
    
    j["powerups"] = json::array();
    for (const auto& powerup : config.powerups) {
        json powerup_json = {
            {"powerup_type", powerup.powerup_type},
            {"spawn_chance", powerup.spawn_chance}
        };
        if (powerup.spawn_on_wave) {
            powerup_json["spawn_on_wave"] = *powerup.spawn_on_wave;
        }
        if (powerup.spawn_condition) {
            powerup_json["spawn_condition"] = *powerup.spawn_condition;
        }
        j["powerups"].push_back(powerup_json);
    }
    
    if (config.max_players) {
        j["max_players"] = *config.max_players;
    }
    if (config.time_limit) {
        j["time_limit"] = *config.time_limit;
    }
    if (config.lives) {
        j["lives"] = *config.lives;
    }
    
    return j.dump(2);
}

}
