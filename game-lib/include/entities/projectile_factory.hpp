#pragma once

#include "ecs/registry.hpp"
#include "ecs/entity.hpp"
#include "components/logic_components.hpp"
#include "components/game_components.hpp"

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage,
                        WeaponUpgradeType upgrade_type = WeaponUpgradeType::None,
                        bool power_cannon_active = false, bool is_drone_projectile = false);

entity createEnemyProjectile(registry& reg, float x, float y, float vx, float vy, int damage = 15);

entity createEnemy2Projectile(registry& reg, float x, float y, float vx, float vy, int damage = 20);

entity createCustomProjectile(registry& reg, float x, float y, float vx, float vy, int damage,
                               const custom_attack_config& config);

entity createEnemy3Projectile(registry& reg, float x, float y, float vx, float vy, int damage = 25, int projectile_type = 0);
entity createFlyingEnemyProjectile(registry& reg, float x, float y, float vx, float vy, int damage = 20);


entity createEnemy4Projectile(registry& reg, float x, float y, float vx, float vy, int damage = 20);

entity createEnemy5Projectile(registry& reg, float x, float y, float vx, float vy, int damage = 30);

entity createExplosiveGrenade(registry& reg, float x, float y, float vx, float vy,
                              float lifetime = 2.0f, float explosion_radius = 80.0f,
                              int explosion_damage = 40);
