#pragma once

#include "LevelConfig.hpp"
#include <ecs/registry.hpp>
#include <functional>

namespace rtype::level {

class LevelEntityFactory {
public:
    using EntityCreatedCallback = std::function<void(entity)>;
    
    explicit LevelEntityFactory(registry& reg);
    
    entity createEnemyFromConfig(const EnemyConfig& config, float x, float y);
    entity createProjectileFromConfig(const ProjectileConfig& config, float x, float y, float dir_x, float dir_y);
    
    void setOnEntityCreated(EntityCreatedCallback callback);
    
private:
    void setupSpriteComponent(entity e, const SpriteConfig& config);
    void setupAnimationComponent(entity e, const SpriteConfig& config);
    void setupBehaviorComponent(entity e, const BehaviorConfig& config);
    void setupAttackComponent(entity e, const AttackPatternConfig& config);
    
    registry& registry_;
    EntityCreatedCallback on_entity_created_;
};

}
