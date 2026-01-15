#include "level/LevelEntityFactory.hpp"
#include "components/game_components.hpp"
#include "ecs/components.hpp"

namespace rtype::level {

LevelEntityFactory::LevelEntityFactory(registry& reg)
    : registry_(reg) {}

void LevelEntityFactory::setOnEntityCreated(EntityCreatedCallback callback) {
    on_entity_created_ = std::move(callback);
}

void LevelEntityFactory::setupSpriteComponent(entity e, const SpriteConfig& config) {
    sprite_component sprite;
    sprite.texture_path = config.texture_path;
    sprite.texture_rect_x = 0;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = config.frame_width;
    sprite.texture_rect_h = config.frame_height;
    sprite.scale = config.scale_x;
    sprite.flip_horizontal = false;
    sprite.visible = true;
    sprite.grayscale = false;
    registry_.add_component(e, sprite);
}

void LevelEntityFactory::setupAnimationComponent(entity e, const SpriteConfig& config) {
    animation_component anim;
    for (int i = 0; i < config.frame_count; ++i) {
        anim.frames.push_back(sf::IntRect(
            i * config.frame_width, 
            0, 
            config.frame_width, 
            config.frame_height
        ));
    }
    anim.current_frame = 0;
    anim.frame_duration = config.frame_duration;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    registry_.add_component(e, anim);
}

void LevelEntityFactory::setupBehaviorComponent(entity e, const BehaviorConfig& config) {
    (void)e;
    (void)config;
}

void LevelEntityFactory::setupAttackComponent(entity e, const AttackPatternConfig& config) {
    if (config.type != "none") {
        weapon weap;
        weap.fire_rate = 1.0f / config.cooldown;
        weap.projectile_speed = config.projectile.speed;
        weap.damage = config.projectile.damage;
        registry_.add_component(e, weap);
    }
}

entity LevelEntityFactory::createEnemyFromConfig(const EnemyConfig& config, float x, float y) {
    entity enemy = registry_.spawn_entity();
    
    registry_.add_component(enemy, position{x, y});
    
    float vel_x = -config.speed;
    float vel_y = 0.0f;
    
    if (config.behavior.type == "sine_wave") {
        vel_y = 0.0f;
    }
    
    registry_.add_component(enemy, velocity{vel_x, vel_y});
    registry_.add_component(enemy, health{config.health});
    
    setupSpriteComponent(enemy, config.sprite);
    setupAnimationComponent(enemy, config.sprite);
    setupBehaviorComponent(enemy, config.behavior);
    setupAttackComponent(enemy, config.attack);
    
    float collision_w = static_cast<float>(config.sprite.frame_width) * config.sprite.scale_x * 0.9f;
    float collision_h = static_cast<float>(config.sprite.frame_height) * config.sprite.scale_y * 0.9f;
    registry_.add_component(enemy, collision_box{collision_w, collision_h});
    
    registry_.add_component(enemy, damage_on_contact{config.damage, false});
    registry_.add_component(enemy, enemy_tag{});
    registry_.add_component(enemy, entity_tag{RType::EntityType::Enemy});
    
    if (on_entity_created_) {
        on_entity_created_(enemy);
    }
    
    return enemy;
}

entity LevelEntityFactory::createProjectileFromConfig(
    const ProjectileConfig& config, float x, float y, float dir_x, float dir_y) {
    
    entity projectile = registry_.spawn_entity();
    
    registry_.add_component(projectile, position{x, y});
    registry_.add_component(projectile, velocity{dir_x * config.speed, dir_y * config.speed});
    
    setupSpriteComponent(projectile, config.sprite);
    if (config.sprite.frame_count > 1) {
        setupAnimationComponent(projectile, config.sprite);
    }
    
    float collision_w = static_cast<float>(config.sprite.frame_width) * config.sprite.scale_x * 0.8f;
    float collision_h = static_cast<float>(config.sprite.frame_height) * config.sprite.scale_y * 0.8f;
    registry_.add_component(projectile, collision_box{collision_w, collision_h});
    
    registry_.add_component(projectile, projectile_tag{});
    registry_.add_component(projectile, entity_tag{RType::EntityType::Projectile});
    
    if (config.homing) {
        homing_component homing;
        homing.speed = config.speed;
        homing.turn_rate = config.homing_strength;
        registry_.add_component(projectile, homing);
    }
    
    if (on_entity_created_) {
        on_entity_created_(projectile);
    }
    
    return projectile;
}

}
