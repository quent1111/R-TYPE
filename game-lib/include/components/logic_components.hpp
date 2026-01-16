#pragma once

#include <cmath>
#include "../../../src/Common/Opcodes.hpp"
#include "../../../engine/ecs/entity.hpp"
#include "../powerup/PowerupRegistry.hpp"
#include "../powerup/PlayerPowerups.hpp"


struct health {
    int current;
    int maximum;

    constexpr health(int max_hp = 100) noexcept : current(max_hp), maximum(max_hp) {}
    constexpr health(int curr_hp, int max_hp) noexcept : current(curr_hp), maximum(max_hp) {}

    [[nodiscard]] constexpr bool is_alive() const noexcept { return current > 0; }
    [[nodiscard]] constexpr bool is_dead() const noexcept { return current <= 0; }
    [[nodiscard]] constexpr float health_percentage() const noexcept {
        return maximum > 0 ? static_cast<float>(current) / static_cast<float>(maximum) : 0.0f;
    }
};

struct damage {
    int amount;
    constexpr explicit damage(int dmg_amount = 10) noexcept : amount(dmg_amount) {}
};

struct damage_on_contact {
    int damage_amount;
    bool destroy_on_hit;

    constexpr damage_on_contact(int dmg = 10, bool destroy = true) noexcept
        : damage_amount(dmg), destroy_on_hit(destroy) {}
};

struct collision_box {
    float width;
    float height;
    float offset_x;
    float offset_y;
    bool enabled;

    constexpr collision_box(float w = 50.0f, float h = 50.0f, float ox = 0.0f,
                            float oy = 0.0f, bool is_enabled = true) noexcept
        : width(w), height(h), offset_x(ox), offset_y(oy), enabled(is_enabled) {}
};

struct multi_hitbox {
    struct hitbox_part {
        float width;
        float height;
        float offset_x;
        float offset_y;
        
        constexpr hitbox_part(float w = 50.0f, float h = 50.0f, float ox = 0.0f, float oy = 0.0f) noexcept
            : width(w), height(h), offset_x(ox), offset_y(oy) {}
    };
    
    std::vector<hitbox_part> parts;
    
    multi_hitbox() = default;
    explicit multi_hitbox(std::vector<hitbox_part> hitbox_parts)
        : parts(std::move(hitbox_parts)) {}
};

struct controllable {
    float speed;
    constexpr explicit controllable(float move_speed = 200.0f) noexcept : speed(move_speed) {}
};

enum class WeaponUpgradeType : uint8_t {
    None = 0,
    PowerShot = 1,
    TripleShot = 2,
    AllyMissile = 3
};

struct weapon {
    float fire_rate;
    float time_since_shot;
    float projectile_speed;
    int damage;
    WeaponUpgradeType upgrade_type;

    constexpr weapon(float rate = 0.5f, float proj_speed = 500.0f, int dmg = 10, 
                     WeaponUpgradeType upgrade = WeaponUpgradeType::None) noexcept
        : fire_rate(rate), time_since_shot(0.0f), projectile_speed(proj_speed), 
          damage(dmg), upgrade_type(upgrade) {}

    constexpr void update(float dt) noexcept { time_since_shot += dt; }

    [[nodiscard]] constexpr bool can_shoot() const noexcept {
        return time_since_shot >= (1.0f / fire_rate);
    }

    constexpr void reset_shot_timer() noexcept { time_since_shot = 0.0f; }

    constexpr void apply_upgrade(WeaponUpgradeType new_upgrade) noexcept {
        upgrade_type = new_upgrade;
        if (new_upgrade == WeaponUpgradeType::PowerShot) {
            damage = 25;
        } else if (new_upgrade == WeaponUpgradeType::TripleShot) {
            fire_rate = 0.6f;
        }
    }
};

struct multishot {
    int extra_projectiles = 0;
    
    constexpr multishot(int extra = 0) noexcept : extra_projectiles(extra) {}
};

struct laser_beam {
    bool active = false;
    float duration = 0.0f;
    float max_duration = 0.0f;
    float damage_per_second = 0.0f;
    float damage_timer = 0.0f;
    int level = 0;
    std::optional<entity> laser_entity;
    
    constexpr laser_beam() noexcept = default;
    
    constexpr laser_beam(float max_dur, float dps, int lvl) noexcept
        : active(false), duration(0.0f), max_duration(max_dur), 
          damage_per_second(dps), damage_timer(0.0f), level(lvl), laser_entity(std::nullopt) {}
    
    constexpr void activate() noexcept {
        active = true;
        duration = max_duration;
        damage_timer = 0.0f;
    }
    
    constexpr void update(float dt) noexcept {
        if (active) {
            duration -= dt;
            damage_timer += dt;
            if (duration <= 0.0f) {
                active = false;
                duration = 0.0f;
            }
        }
    }
    
    [[nodiscard]] constexpr bool can_damage() const noexcept {
        return active && damage_timer >= 0.1f;
    }
    
    constexpr void reset_damage_timer() noexcept {
        damage_timer = 0.0f;
    }
};

struct bounded_movement {
    float min_x, max_x;
    float min_y, max_y;

    constexpr bounded_movement(float minx = 0.0f, float maxx = 1920.0f, float miny = 0.0f,
                                float maxy = 1080.0f) noexcept
        : min_x(minx), max_x(maxx), min_y(miny), max_y(maxy) {}
};

struct wave_manager {
    float timer = 0.0f;
    float spawn_interval = 5.0f;
    int enemies_per_wave = 3;
    
    constexpr wave_manager(float interval = 5.0f, int count = 3) noexcept 
        : spawn_interval(interval), enemies_per_wave(count) {}
};

struct level_manager {
    int current_level = 1;
    int enemies_killed_this_level = 0;
    int enemies_needed_for_next_level = 1;
    bool awaiting_upgrade_choice = false;
    bool level_completed = false;
    bool is_custom_level = false;
    float level_start_delay = 3.0f;
    float level_start_timer = 0.0f;
    
    constexpr level_manager() noexcept = default;
    
    constexpr void on_enemy_killed() noexcept {
        enemies_killed_this_level++;
        if (enemies_killed_this_level >= enemies_needed_for_next_level) {
            level_completed = true;
            awaiting_upgrade_choice = true;
        }
    }
    
    constexpr void advance_to_next_level() noexcept {
        current_level++;
        enemies_killed_this_level = 0;
        level_completed = false;
        awaiting_upgrade_choice = false;
        enemies_needed_for_next_level = current_level;
        level_start_timer = 0.0f;
    }
    
    [[nodiscard]] constexpr int get_progress_percentage() const noexcept {
        if (enemies_needed_for_next_level == 0) return 0;
        return (enemies_killed_this_level * 100) / enemies_needed_for_next_level;
    }
    
    [[nodiscard]] constexpr bool is_level_intro_active() const noexcept {
        return level_start_timer < level_start_delay;
    }
    
    constexpr void update_intro_timer(float dt) noexcept {
        if (level_start_timer < level_start_delay) {
            level_start_timer += dt;
        }
    }
};

enum class PowerUpType : uint8_t {
    None = 0,
    PowerCannon = 1,
    Shield = 2,
    LittleFriend = 3
};

struct power_cannon {
    bool active = false;
    float duration = 10.0f;
    float time_remaining = 0.0f;
    int damage = 50;
    float fire_rate = 3.0f;

    constexpr power_cannon() noexcept = default;

    constexpr void activate(float custom_duration = -1.0f, int custom_damage = -1) noexcept {
        active = true;
        if (custom_duration >= 0.0f) {
            duration = custom_duration;
        }
        time_remaining = duration;
        if (custom_damage >= 0) {
            damage = custom_damage;
        }
    }
    
    constexpr void update(float dt) noexcept {
        if (active) {
            time_remaining -= dt;
            if (time_remaining <= 0.0f) {
                active = false;
                time_remaining = 0.0f;
            }
        }
    }
    
    [[nodiscard]] constexpr bool is_active() const noexcept { return active; }
    [[nodiscard]] constexpr float get_remaining_percentage() const noexcept {
        return duration > 0.0f ? (time_remaining / duration) : 0.0f;
    }
};

struct shield {
    bool active = false;
    float duration = 10.0f;
    float time_remaining = 0.0f;
    float radius = 80.0f;
    
    constexpr shield() noexcept = default;
    
    constexpr void activate(float custom_duration = -1.0f, float custom_radius = -1.0f) noexcept {
        active = true;
        if (custom_duration >= 0.0f) {
            duration = custom_duration;
        }
        time_remaining = duration;
        if (custom_radius >= 0.0f) {
            radius = custom_radius;
        }
    }
    
    constexpr void update(float dt) noexcept {
        if (active) {
            time_remaining -= dt;
            if (time_remaining <= 0.0f) {
                active = false;
                time_remaining = 0.0f;
            }
        }
    }
    
    [[nodiscard]] constexpr bool is_active() const noexcept { return active; }
    [[nodiscard]] constexpr float get_remaining_percentage() const noexcept {
        return duration > 0.0f ? (time_remaining / duration) : 0.0f;
    }
    
    [[nodiscard]] constexpr bool is_enemy_in_range(float enemy_x, float enemy_y, 
                                                     float player_x, float player_y) const noexcept {
        if (!active) return false;
        float dx = enemy_x - player_x;
        float dy = enemy_y - player_y;
        float dist_squared = dx * dx + dy * dy;
        return dist_squared <= (radius * radius);
    }
};

struct little_friend {
    bool active = false;
    float duration = 10.0f;
    float time_remaining = 0.0f;
    std::vector<std::optional<entity>> friend_entities;
    int num_drones = 1;
    int damage = 15;
    float fire_rate = 2.0f;
    float shoot_timer = 0.0f;
    float oscillation_timer = 0.0f;
    float oscillation_speed = 2.0f;
    float oscillation_amplitude = 15.0f;

    bool entry_animation_complete = false;
    float entry_animation_timer = 0.0f;
    float entry_animation_duration = 1.0f;
    
    bool exit_animation_started = false;
    float exit_animation_timer = 0.0f;
    float exit_animation_duration = 1.0f;

    constexpr little_friend() noexcept = default;
    
    constexpr void activate() noexcept {
        active = true;
        time_remaining = duration;
        shoot_timer = 0.0f;
        entry_animation_complete = false;
        entry_animation_timer = 0.0f;
        exit_animation_started = false;
        exit_animation_timer = 0.0f;
    }
    
    constexpr void update(float dt) noexcept {
        if (active) {
            time_remaining -= dt;
            shoot_timer += dt;
            oscillation_timer += dt;
            
            if (!entry_animation_complete) {
                entry_animation_timer += dt;
                if (entry_animation_timer >= entry_animation_duration) {
                    entry_animation_complete = true;
                }
            }
            
            if (time_remaining <= exit_animation_duration && !exit_animation_started) {
                exit_animation_started = true;
                exit_animation_timer = 0.0f;
            }
            
            if (exit_animation_started) {
                exit_animation_timer += dt;
            }
            
            if (time_remaining <= 0.0f) {
                active = false;
                time_remaining = 0.0f;
            }
        }
    }
    
    [[nodiscard]] constexpr bool is_active() const noexcept { return active; }
    [[nodiscard]] constexpr float get_remaining_percentage() const noexcept {
        return duration > 0.0f ? (time_remaining / duration) : 0.0f;
    }
    
    [[nodiscard]] float get_entry_progress() const noexcept {
        if (entry_animation_complete) return 1.0f;
        return entry_animation_timer / entry_animation_duration;
    }
    
    [[nodiscard]] float get_exit_progress() const noexcept {
        if (!exit_animation_started) return 0.0f;
        return exit_animation_timer / exit_animation_duration;
    }
    
    [[nodiscard]] constexpr bool can_shoot() const noexcept {
        return active && shoot_timer >= fire_rate;
    }
    
    [[nodiscard]] float get_vertical_offset() const noexcept {
        return std::sin(oscillation_timer * oscillation_speed) * oscillation_amplitude;
    }
    
    constexpr void reset_shoot_timer() noexcept {
        shoot_timer = 0.0f;
    }
};

struct missile_drone {
    bool active = false;
    std::vector<std::optional<entity>> drone_entities;
    int num_drones = 1;
    int missiles_per_volley = 5;
    float fire_rate = 3.0f;
    float shoot_timer = 0.0f;
    float oscillation_timer = 0.0f;
    float oscillation_speed = 2.0f;
    float oscillation_amplitude = 15.0f;
    
    constexpr missile_drone() noexcept = default;
    
    constexpr void activate(int drones, int missiles) noexcept {
        active = true;
        num_drones = drones;
        missiles_per_volley = missiles;
        shoot_timer = 0.0f;
    }
    
    constexpr void update(float dt) noexcept {
        if (active) {
            shoot_timer += dt;
            oscillation_timer += dt;
        }
    }
    
    [[nodiscard]] constexpr bool is_active() const noexcept { return active; }
    
    [[nodiscard]] constexpr bool can_shoot() const noexcept {
        return active && shoot_timer >= fire_rate;
    }
    
    [[nodiscard]] float get_vertical_offset() const noexcept {
        return std::sin(oscillation_timer * oscillation_speed) * oscillation_amplitude;
    }
    
    constexpr void reset_shoot_timer() noexcept {
        shoot_timer = 0.0f;
    }
};

struct player_tag {};
struct enemy_tag {};
struct boss_tag {};
struct projectile_tag {};
struct ally_projectile_tag {};
struct explosion_tag {
    float lifetime;
    float elapsed;

    constexpr explosion_tag(float life = 0.5f) noexcept
        : lifetime(life), elapsed(0.0f) {}
};

struct entity_tag {
    RType::EntityType type;
    constexpr explicit entity_tag(RType::EntityType t_type) noexcept : type(t_type) {}
};

struct network_id {
    int client_id;
    constexpr explicit network_id(int c_id = -1) noexcept : client_id(c_id) {}
};

using player_powerups_component = powerup::PlayerPowerups;


enum class SerpentPartType : uint8_t {
    Nest = 0,
    Head = 1,
    Body = 2,
    Scale = 3,
    Tail = 4
};

struct serpent_part {
    SerpentPartType part_type;
    int part_index;
    std::optional<entity> parent_entity;
    std::optional<entity> boss_entity;
    std::optional<entity> attached_body;
    bool can_attack = false;
    float attack_cooldown = 0.0f;
    float attack_timer = 0.0f;
    
    float target_x = 0.0f;
    float target_y = 0.0f;
    float follow_delay = 0.1f;
    float follow_timer = 0.0f;
    
    float rotation = 0.0f;
    
    serpent_part() = default;
    serpent_part(SerpentPartType type, int index)
        : part_type(type), part_index(index) {
        can_attack = (type == SerpentPartType::Scale);
        if (can_attack) {
            attack_cooldown = 2.0f;
        }
    }
};

struct serpent_boss_controller {
    int total_health = 5000;
    int current_health = 5000;
    int num_body_parts = 12;
    int num_scale_parts = 3;
    float cycle_multiplier = 1.0f;

    bool spawn_complete = false;
    bool nest_visible = false;
    float spawn_timer = 0.0f;
    float nest_rise_duration = 2.0f;
    float serpent_emerge_duration = 4.0f;
    int parts_spawned = 0;
    
    float movement_timer = 0.0f;
    float movement_speed = 400.0f;
    float wave_frequency = 2.0f;
    float wave_amplitude = 60.0f;
    float direction_change_timer = 0.0f;
    float target_x = 960.0f;
    float target_y = 500.0f;
    int current_waypoint_idx = 0;
    int previous_waypoint_idx = -1;

    float scale_shoot_timer = 0.0f;
    float scale_shoot_cooldown = 1.2f;
    int current_scale_index = 0;

    float scream_timer = 0.0f;
    float scream_cooldown = 15.0f;
    bool scream_active = false;
    float scream_duration = 1.5f;
    float scream_elapsed = 0.0f;

    float laser_timer = 0.0f;
    float laser_cooldown = 20.0f;
    bool laser_charging = false;
    bool laser_firing = false;
    float laser_charge_duration = 2.0f;
    float laser_fire_duration = 2.5f;
    float laser_elapsed = 0.0f;
    float laser_angle = 0.0f;
    float laser_start_angle = 0.0f;
    float laser_sweep_direction = 1.0f;

    std::optional<entity> nest_entity;
    std::optional<entity> head_entity;
    std::vector<entity> body_entities;
    std::vector<entity> scale_entities;
    std::optional<entity> tail_entity;
    std::optional<entity> laser_entity;

    serpent_boss_controller() = default;
    serpent_boss_controller(int hp, int body_count = 12, int scale_count = 3)
        : total_health(hp), current_health(hp),
          num_body_parts(body_count), num_scale_parts(scale_count) {}

    void take_global_damage(int damage) {
        current_health -= damage;
        if (current_health < 0) current_health = 0;
    }

    [[nodiscard]] bool is_defeated() const { return current_health <= 0; }
    [[nodiscard]] float health_percentage() const {
        return total_health > 0 ? static_cast<float>(current_health) / static_cast<float>(total_health) : 0.0f;
    }
};

struct serpent_nest_tag {};

enum class CompilerState : uint8_t {
    Entering = 0,
    Assembled = 1,
    Splitting = 2,
    Separated = 3,
    Merging = 4
};

struct compiler_boss_controller {
    int total_health = 3000;
    int current_health = 3000;
    float cycle_multiplier = 1.0f;

    CompilerState state = CompilerState::Entering;
    float state_timer = 0.0f;

    float assembled_duration = 8.0f;
    float split_duration = 2.0f;
    float separated_duration = 15.0f;
    float merge_duration = 2.0f;

    float target_x = 1100.0f;
    float target_y = 400.0f;
    float movement_speed = 120.0f;

    float min_x = 900.0f;
    float max_x = 1600.0f;
    float min_y = 150.0f;
    float max_y = 750.0f;

    float follow_offset_x = 400.0f;
    float follow_smoothing = 2.0f;

    float light_timer = 0.0f;
    float light_duration = 0.5f;
    bool light_on = true;

    std::optional<entity> part1_entity;
    std::optional<entity> part2_entity;
    std::optional<entity> part3_entity;

    float part1_target_x = 0.0f, part1_target_y = 0.0f;
    float part2_target_x = 0.0f, part2_target_y = 0.0f;
    float part3_target_x = 0.0f, part3_target_y = 0.0f;

    float part_movement_timer = 0.0f;
    float part_movement_interval = 1.5f;

    float attack_timer = 0.0f;
    float attack_cooldown = 1.2f;
    int attack_pattern = 0;

    float special_attack_timer = 0.0f;
    float special_attack_cooldown = 6.0f;
    bool charging_special = false;
    float charge_time = 0.0f;
    float charge_duration = 1.5f;

    float part1_attack_timer = 0.0f;
    float part2_attack_timer = 0.0f;
    float part3_attack_timer = 0.0f;

    float part1_death_timer = -1.0f;
    float part2_death_timer = -1.0f;
    float part3_death_timer = -1.0f;
    float death_delay = 0.3f;

    bool entrance_complete = false;
    float entrance_target_x = 1100.0f;

    compiler_boss_controller() = default;
    compiler_boss_controller(int hp) : total_health(hp), current_health(hp) {}

    void take_damage(int damage) {
        current_health -= damage;
        if (current_health < 0) current_health = 0;
    }

    [[nodiscard]] bool is_defeated() const {
        return !part1_entity.has_value() && !part2_entity.has_value() && !part3_entity.has_value();
    }
    [[nodiscard]] float health_percentage() const {
        return total_health > 0 ? static_cast<float>(current_health) / static_cast<float>(total_health) : 0.0f;
    }
};

struct compiler_part_tag {
    int part_index;
    compiler_part_tag(int idx = 1) : part_index(idx) {}
};

struct position_history {
    static constexpr size_t MAX_HISTORY = 60;
    std::vector<std::pair<float, float>> positions;
    size_t current_index = 0;

    position_history() {
        positions.resize(MAX_HISTORY, {0.0f, 0.0f});
    }

    void add_position(float x, float y) {
        positions[current_index] = {x, y};
        current_index = (current_index + 1) % MAX_HISTORY;
    }

    std::pair<float, float> get_delayed_position(int frames_delay) const {
        if (frames_delay >= static_cast<int>(MAX_HISTORY)) {
            frames_delay = MAX_HISTORY - 1;
        }
        size_t index = (current_index + MAX_HISTORY - static_cast<size_t>(frames_delay)) % MAX_HISTORY;
        return positions[index];
    }
};

struct game_settings {
    bool friendly_fire_enabled = false;
    float difficulty_multiplier = 1.0f;
    
    constexpr game_settings() noexcept = default;
    constexpr explicit game_settings(bool ff_enabled, float diff_mult = 1.0f) noexcept 
        : friendly_fire_enabled(ff_enabled), difficulty_multiplier(diff_mult) {}
};
