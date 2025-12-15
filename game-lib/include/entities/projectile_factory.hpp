#pragma once

#include "ecs/registry.hpp"
#include "ecs/entity.hpp"
#include "components/logic_components.hpp"

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage, 
                        WeaponUpgradeType upgrade_type = WeaponUpgradeType::None);

entity createEnemyProjectile(registry& reg, float x, float y, float vx, float vy, int damage = 15);

entity createEnemy2Projectile(registry& reg, float x, float y, float vx, float vy, int damage = 20);
