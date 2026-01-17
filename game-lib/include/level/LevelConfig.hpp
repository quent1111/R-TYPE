#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace rtype::level {

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
    std::string type = "basic";
    SpriteConfig sprite;
    float speed = 400.0f;
    int damage = 10;
    float fire_rate = 1.0f;
    bool homing = false;
    float homing_strength = 0.0f;
};

struct MovementPatternConfig {
    std::string type = "linear";
    float amplitude = 0.0f;
    float frequency = 0.0f;
    float phase = 0.0f;
    std::vector<std::pair<float, float>> waypoints;
};

struct BehaviorConfig {
    std::string type = "straight";
    MovementPatternConfig movement;
    bool tracks_player = false;
    float tracking_speed = 0.0f;
    float aggro_range = 0.0f;
};

struct AttackPatternConfig {
    std::string type = "none";
    float cooldown = 2.0f;
    int burst_count = 1;
    float burst_delay = 0.1f;
    float spread_angle = 30.0f;
    int projectile_count = 1;
    bool aim_at_player = false;
    ProjectileConfig projectile;
};

struct EnemyConfig {
    std::string id;
    std::string name;
    SpriteConfig sprite;
    int health = 100;
    float speed = 100.0f;
    int damage = 10;
    int score_value = 100;
    BehaviorConfig behavior;
    AttackPatternConfig attack;
    std::optional<std::string> death_sound;
    std::optional<SpriteConfig> death_animation;
};

struct SpawnPointConfig {
    float x = 0.0f;
    float y = 0.0f;
    std::string position_type = "absolute";
    float offset_x = 0.0f;
    float offset_y = 0.0f;
};

struct EnemySpawnConfig {
    std::string enemy_id;
    int count = 1;
    float spawn_delay = 0.5f;
    SpawnPointConfig spawn_point;
    std::optional<std::string> formation;
};

struct WaveConfig {
    int wave_number = 0;
    std::string name;
    std::vector<EnemySpawnConfig> enemies;
    float wave_delay = 2.0f;
    bool is_boss_wave = false;
    std::optional<std::string> trigger_condition;
    std::optional<std::string> music_override;
};

struct PowerupSpawnConfig {
    std::string powerup_type;
    float spawn_chance = 0.1f;
    std::optional<int> spawn_on_wave;
    std::optional<std::string> spawn_condition;
};

struct EnvironmentConfig {
    std::string background_texture;
    std::optional<std::string> parallax_layer_1;
    std::optional<std::string> parallax_layer_2;
    float scroll_speed = 50.0f;
    bool scroll_infinite = true;
    bool background_static = false;
    std::optional<std::string> music;
    std::optional<std::string> ambient_sound;
};

struct LevelMetadata {
    std::string id;
    std::string name;
    std::string author;
    std::string version = "1.0.0";
    std::string description;
    int difficulty = 1;
    std::optional<std::string> preview_image;
};

struct LevelConfig {
    LevelMetadata metadata;
    EnvironmentConfig environment;
    std::unordered_map<std::string, EnemyConfig> enemy_definitions;
    std::vector<WaveConfig> waves;
    std::vector<PowerupSpawnConfig> powerups;
    std::optional<int> max_players = 4;
    std::optional<float> time_limit;
    std::optional<int> lives = 3;
};

}
