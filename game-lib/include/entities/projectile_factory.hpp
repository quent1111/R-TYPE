#pragma once

#include "ecs/registry.hpp"
#include "ecs/entity.hpp"

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage);
