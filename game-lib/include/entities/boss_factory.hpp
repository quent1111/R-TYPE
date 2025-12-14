#pragma once

#include "../../../engine/ecs/registry.hpp"

// Create a boss entity at a given position
entity createBoss(registry& reg, float x, float y, int boss_type = 1);
