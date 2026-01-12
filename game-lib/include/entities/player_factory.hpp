#pragma once

#include "ecs/registry.hpp"
#include "ecs/entity.hpp"

entity createPlayer(registry& reg, float x, float y, int player_index = 0);
