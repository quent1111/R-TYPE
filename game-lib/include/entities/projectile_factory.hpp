#pragma once

#include "ecs/registry.hpp"
#include "ecs/entity.hpp"
#include "components/logic_components.hpp"

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage, 
                        WeaponUpgradeType upgrade_type = WeaponUpgradeType::None);
