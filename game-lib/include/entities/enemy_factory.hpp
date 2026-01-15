#pragma once

#include "ecs/registry.hpp"
#include "ecs/entity.hpp"

entity createBasicEnemy(registry& reg, float x, float y);

entity createSecondaryEnemy(registry& reg, float x, float y);

entity createFlyingEnemy(registry& reg, float x, float y);

entity createWaveEnemy(registry& reg, float x, float y);

entity createTankEnemy(registry& reg, float x, float y);

void spawnEnemyWave(registry& reg, int count = 5, int level = 1);
