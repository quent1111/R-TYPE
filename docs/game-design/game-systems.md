# Game Systems Guide

This document describes the main game systems used in R-TYPE and how they interact with each other.

---

## Systems Architecture

Game systems are organized into **managers** that encapsulate the server's business logic. Each manager has a unique responsibility and interacts with the **ECS Registry** to manipulate entities and components.

```
┌────────────────────────────────────────────────┐
│            GameSession (Orchestrator)           │
│                                                 │
│  ┌───────────────┐  ┌───────────────┐         │
│  │PlayerManager  │  │ LevelManager  │         │
│  └───────────────┘  └───────────────┘         │
│                                                 │
│  ┌───────────────┐  ┌───────────────┐         │
│  │ BossManager   │  │ Registry ECS  │         │
│  └───────────────┘  └───────────────┘         │
└────────────────────────────────────────────────┘
```

---

## 1. PlayerManager

**Responsibility:** Managing player lifecycle (creation, deletion, respawn, state checking).

### Main Methods

#### `create_player()`
Creates a new player in the ECS registry and associates it with a network client.

```cpp
entity create_player(registry& reg, 
                    std::unordered_map<int, std::size_t>& client_entity_ids,
                    int client_id, 
                    float start_x = 100.0f, 
                    float start_y = 100.0f);
```

**Usage example:**
```cpp
// In GameSession, when a client joins
auto player_entity = _player_manager.create_player(
    _registry, 
    _client_entity_ids, 
    client_id, 
    100.0f, 
    300.0f
);
```

**Components added:**
- `position` - Player's initial position
- `velocity` - Movement speed
- `health` - Hit points (100/100)
- `network_id` - Network client ID
- `player_tag` - Marker to identify players
- `controllable` - Enables player control
- `drawable` - Enables visual rendering

---

#### `remove_player()`
Removes a player from the game and cleans up associated resources.

```cpp
void remove_player(registry& reg, 
                  std::unordered_map<int, std::size_t>& client_entity_ids,
                  int client_id);
```

**Usage:**
```cpp
// When a client disconnects
_player_manager.remove_player(_registry, _client_entity_ids, client_id);
```

---

#### `check_all_players_dead()`
Checks if all players are dead (Game Over).

```cpp
bool check_all_players_dead(registry& reg,
                           const std::unordered_map<int, std::size_t>& client_entity_ids);
```

**Usage:**
```cpp
if (_player_manager.check_all_players_dead(_registry, _client_entity_ids)) {
    _game_broadcaster.broadcast_game_over(server);
    reset_game_state();
}
```

---

#### `respawn_dead_players()`
Resets all dead players with full hit points (used at the start of a new level).

```cpp
void respawn_dead_players(registry& reg,
                         std::unordered_map<int, std::size_t>& client_entity_ids);
```

**Usage:**
```cpp
// At the start of a new level
_player_manager.respawn_dead_players(_registry, _client_entity_ids);
```

---

## 2. LevelManager

**Responsibility:** Managing level progression, completion detection, cleanup between levels.

### Main Methods

#### `check_level_completion()`
Checks if the current level is completed (all enemies defeated).

```cpp
bool check_level_completion(registry& reg);
```

**Usage:**
```cpp
if (_level_manager.check_level_completion(_registry)) {
    // Transition to the next level
    transition_to_next_level(server);
}
```

**Internal logic:**
- Iterates through all `level_manager` components in the registry
- Returns `true` if `level_completed` is marked

---

#### `advance_level()`
Advances to the next level and resets the level manager state.

```cpp
void advance_level(registry& reg);
```

**Usage:**
```cpp
// After completing a level
_level_manager.advance_level(_registry);
_game_broadcaster.broadcast_level_info(server, new_level);
```

**Effects:**
- Increments `current_level`
- Resets `level_completed` to `false`
- Logs the new level

---

#### `clear_enemies_and_projectiles()`
Cleans up all enemies, projectiles, bosses, and homing entities between levels.

```cpp
void clear_enemies_and_projectiles(registry& reg, std::optional<entity>& boss_entity);
```

**Usage:**
```cpp
// Before starting a new level
_level_manager.clear_enemies_and_projectiles(_registry, _boss_entity);
```

**Entities cleaned:**
- `enemy_tag` - Regular enemies
- `projectile_tag` - Enemy projectiles
- `boss_tag` - Bosses
- `homing_component` - Homing enemies

---

#### `get_current_level()`
Returns the current level number (1-5).

```cpp
uint8_t get_current_level(registry& reg);
```

**Usage:**
```cpp
uint8_t current_level = _level_manager.get_current_level(_registry);
if (current_level == 5) {
    // Spawn boss
    _boss_manager.spawn_boss_level_5(...);
}
```

---

## 3. BossManager

**Responsibility:** Managing boss AI (spawn, behavior, attacks, homing enemies).

### Main Methods

#### `spawn_boss_level_5()`
Spawns the level 5 boss with an entrance animation from the right.

```cpp
void spawn_boss_level_5(registry& reg, 
                       std::optional<entity>& boss_entity,
                       float& boss_animation_timer, 
                       float& boss_shoot_timer,
                       bool& boss_animation_complete, 
                       bool& boss_entrance_complete,
                       float& boss_target_x);
```

**Boss characteristics:**
- **Hit points:** 2000 HP
- **Initial position:** X=2400, Y=540 (off-screen to the right)
- **Final position:** X=1500, Y=540
- **Entry speed:** -150 px/s (to the left)
- **Contact damage:** 50 HP

**Multiple hitboxes:**
```cpp
multi_hitbox boss_hitboxes;
boss_hitboxes.parts = {
    {250.0f, 250.0f, -200.0f, -350.0f},  // Head
    {350.0f, 500.0f,   25.0f, -100.0f},  // Body
    {200.0f, 200.0f, -100.0f,  220.0f}   // Tail
};
```

**Components added:**
- `position`, `velocity`, `health`
- `multi_hitbox` - 3 collision zones
- `damage_on_contact` - Contact damage
- `boss_tag`, `entity_tag`, `damage_flash_component`

---

#### `update_boss_behavior()`
Updates the boss AI (entrance animation, shooting, homing enemy spawning).

```cpp
void update_boss_behavior(registry& reg, 
                         std::optional<entity>& boss_entity,
                         const std::unordered_map<int, std::size_t>& client_entity_ids,
                         float& boss_animation_timer, 
                         float& boss_shoot_timer,
                         float boss_shoot_cooldown, 
                         bool& boss_animation_complete,
                         bool& boss_entrance_complete, 
                         float boss_target_x,
                         int& boss_shoot_counter, 
                         float dt);
```

**Behavior phases:**

1. **Entrance phase** (`!boss_entrance_complete`)
   - The boss moves from X=2400 to X=1500
   - Once arrived, velocity is set to zero
   - Transition to animation phase

2. **Animation phase** (`boss_animation_timer < 2.5s`)
   - Duration: 2.5 seconds
   - The boss remains stationary
   - Allows the player to see the boss before attacks

3. **Combat phase** (`boss_animation_complete`)
   - Shoots projectiles every `boss_shoot_cooldown` seconds (typically 1.0s)
   - Every 3 shots, spawns a homing enemy

**Usage:**
```cpp
// In game loop (GameSession::run)
_boss_manager.update_boss_behavior(
    _registry, 
    _boss_entity,
    _client_entity_ids,
    _boss_animation_timer,
    _boss_shoot_timer,
    1.0f,  // shoot cooldown
    _boss_animation_complete,
    _boss_entrance_complete,
    _boss_target_x,
    _boss_shoot_counter,
    dt
);
```

---

#### `boss_shoot_projectile()`
The boss shoots projectiles aimed at each living player.

```cpp
void boss_shoot_projectile(registry& reg, 
                          std::optional<entity>& boss_entity,
                          const std::unordered_map<int, std::size_t>& client_entity_ids);
```

**Shooting logic:**
1. Gets the boss position
2. Iterates through all living players (health > 0)
3. Calculates direction to each player
4. Spawns a projectile with 300 px/s velocity

**Projectile created:**
- `position` - Boss position
- `velocity` - Direction to player (normalized × 300)
- `projectile_tag` - Enemy projectile marker
- `entity_tag` - Type EntityType::EnemyProjectile
- `damage_on_contact` - 20 damage

---

#### `boss_spawn_homing_enemy()`
The boss spawns a homing enemy that tracks players.

```cpp
void boss_spawn_homing_enemy(registry& reg, std::optional<entity>& boss_entity);
```

**Homing enemy characteristics:**
- Spawns at boss position
- Speed: 100 px/s
- Hit points: 50 HP
- Damage: 10 HP
- Automatically tracks the nearest player

**Components:**
- `homing_component{100.0f, 200.0f}` - Speed 100, detection 200
- `enemy_tag`, `health{50, 50}`, `damage_on_contact{10, false}`

---

#### `update_homing_enemies()`
Updates the behavior of all homing enemies (chasing the nearest player).

```cpp
void update_homing_enemies(registry& reg,
                          const std::unordered_map<int, std::size_t>& client_entity_ids,
                          float dt);
```

**Usage:**
```cpp
// In game loop
_boss_manager.update_homing_enemies(_registry, _client_entity_ids, dt);
```

**Logic:**
1. For each homing enemy:
   - Find the nearest living player
   - Calculate direction to that player
   - Adjust velocity to track it
   - Constant speed defined by `homing_component.speed`

---

## System Interactions

### Example: Transition to Level 5

```cpp
// 1. Check if level 4 is completed
if (_level_manager.check_level_completion(_registry)) {
    
    // 2. Clean up remaining enemies
    _level_manager.clear_enemies_and_projectiles(_registry, _boss_entity);
    
    // 3. Advance to level 5
    _level_manager.advance_level(_registry);
    
    // 4. Respawn dead players
    _player_manager.respawn_dead_players(_registry, _client_entity_ids);
    
    // 5. Inform clients
    _game_broadcaster.broadcast_level_info(server, 5);
    
    // 6. Wait before spawning boss
    _waiting_for_level_start = true;
    _level_start_timer = 0.0f;
}

// After 3 seconds
if (_level_start_timer >= 3.0f) {
    _boss_manager.spawn_boss_level_5(
        _registry, _boss_entity,
        _boss_animation_timer, _boss_shoot_timer,
        _boss_animation_complete, _boss_entrance_complete,
        _boss_target_x
    );
    _waiting_for_level_start = false;
}
```

---

### Example: Game Over Management

```cpp
// Check if all players are dead
if (_player_manager.check_all_players_dead(_registry, _client_entity_ids)) {
    
    // Send Game Over message to clients
    _game_broadcaster.broadcast_game_over(server);
    
    // Reset game state
    reset_game_state();
}
```

---

## Design Patterns Used

| Pattern | Manager | Description |
|---------|---------|-------------|
| **Single Responsibility** | All | Each manager has a single clear responsibility |
| **Dependency Injection** | All | The `registry` is passed as parameter, not owned |
| **Facade** | GameSession | Orchestrates managers and simplifies the API |
| **Strategy** | BossManager | Different attack strategies (projectiles, homing) |
| **State Machine** | LevelManager | States: in progress, completed, transition |

---

## Best Practices

### 1. Never store references to Registry
```cpp
// ❌ Bad
class PlayerManager {
    registry& _reg;  // DANGER: stored reference
};

// ✅ Good
class PlayerManager {
    void create_player(registry& reg, ...);  // Pass as parameter
};
```

### 2. Always check `std::optional`
```cpp
// ❌ Bad
auto pos = reg.get_component<position>(entity);
float x = pos->x;  // CRASH if pos is nullopt

// ✅ Good
auto pos_opt = reg.get_component<position>(entity);
if (pos_opt.has_value()) {
    float x = pos_opt->x;
}
```

### 3. Clean entities properly
```cpp
// ✅ Correct order
reg.remove_component<entity_tag>(entity);  // First remove the tag
reg.kill_entity(entity);                    // Then kill the entity
```

### 4. Log important events
```cpp
std::cout << "[PlayerManager] Player " << client_id << " spawned" << std::endl;
std::cout << "[LevelManager] Advanced to level " << level << std::endl;
std::cout << "[BossManager] Boss defeated!" << std::endl;
```

---

## Adding a New Manager

To add a new manager (e.g., `PowerupManager`):

1. **Create the header** `server/include/game/PowerupManager.hpp`:
```cpp
#pragma once
#include "../../engine/ecs/registry.hpp"

namespace server {
class PowerupManager {
public:
    void spawn_powerup(registry& reg, float x, float y);
    void collect_powerup(registry& reg, entity player, entity powerup);
};
}
```

2. **Create the implementation** `server/src/game/PowerupManager.cpp`

3. **Add to GameSession**:
```cpp
// include/game/GameSession.hpp
#include "game/PowerupManager.hpp"

class GameSession {
    PowerupManager _powerup_manager;
};
```

4. **Use in game loop**:
```cpp
// In GameSession::run()
_powerup_manager.spawn_powerup(_registry, x, y);
```

---

## Summary

| Manager | Responsibility | Key methods |
|---------|---------------|---------------|
| **PlayerManager** | Player lifecycle | create, remove, respawn, check_dead |
| **LevelManager** | Level progression | check_completion, advance, clear, get_level |
| **BossManager** | Boss AI | spawn, update_behavior, shoot, spawn_homing |

Managers communicate via the **ECS Registry** and are orchestrated by **GameSession**. Each manager is independent and testable in isolation.
