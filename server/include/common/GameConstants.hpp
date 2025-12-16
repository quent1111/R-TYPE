#pragma once

#include <cstdint>

namespace server {

constexpr uint32_t ENEMY_ID_OFFSET = 10000;
constexpr uint32_t PROJECTILE_ID_OFFSET = 20000;
constexpr uint32_t OTHER_ID_OFFSET = 30000;

enum class GamePhase { Lobby, InGame };

}  // namespace server
