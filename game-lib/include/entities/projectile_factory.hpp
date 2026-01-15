#pragma once

#include "ecs/registry.hpp"
#include "ecs/entity.hpp"
#include "components/logic_components.hpp"
#include "components/game_components.hpp"

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage, 
                        WeaponUpgradeType upgrade_type = WeaponUpgradeType::None,
                        bool power_cannon_active = false);

entity createEnemyProjectile(registry& reg, float x, float y, float vx, float vy, int damage = 15);

entity createEnemy2Projectile(registry& reg, float x, float y, float vx, float vy, int damage = 20);

entity createCustomProjectile(registry& reg, float x, float y, float vx, float vy, int damage,
                               const custom_attack_config& config);
