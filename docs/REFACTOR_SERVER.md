# Server Refactoring R-TYPE

## Summary
The server has been refactored to separate responsibilities and reduce the "God Object" `Game.cpp` from **1111 lines → 475 lines**.

---

## New Structure

```
server/
├── include/
│   ├── common/           ← Shared types and constants
│   │   ├── ClientEndpoint.hpp
│   │   ├── GameConstants.hpp
│   │   ├── InputKey.hpp
│   │   ├── NetworkPacket.hpp
│   │   └── SafeQueue.hpp
│   ├── game/             ← Business logic
│   │   ├── BossManager.hpp
│   │   ├── GameSession.hpp
│   │   ├── LevelManager.hpp
│   │   └── PlayerManager.hpp
│   ├── handlers/         ← Client input processing
│   │   ├── InputHandler.hpp
│   │   ├── PowerupHandler.hpp
│   │   └── WeaponHandler.hpp
│   └── network/          ← Network layer
│       ├── EntityBroadcaster.hpp
│       ├── GameBroadcaster.hpp
│       ├── LobbyBroadcaster.hpp
│       ├── PowerupBroadcaster.hpp
│       └── UDPServer.hpp
└── src/
    ├── main.cpp
    ├── game/*.cpp
    ├── handlers/*.cpp
    └── network/*.cpp
```

---

##  Changements Principaux

### 1. Namespace `server::`
All classes are in the namespace `server::`:
```cpp
server::GameSession gameInstance;
server::UDPServer server(io_context, bind_address, port);
```

### 2. Broadcasters (Separate network sending)
```cpp
// Avant: broadcast_entity_positions() dans Game.cpp
// Après: dedicated classes
_entity_broadcaster.broadcast_entity_positions(server, _registry, _client_entity_ids);
_lobby_broadcaster.broadcast_lobby_status(server, _client_ready_status);
_game_broadcaster.broadcast_game_over(server);
_powerup_broadcaster.broadcast_powerup_status(server, _registry, _client_entity_ids);
```

**Broadcasters créés:**
- `EntityBroadcaster` - Entity positions
- `LobbyBroadcaster` - Lobby status
- `GameBroadcaster` - Level info, game over
- `PowerupBroadcaster` - Powerup status

### 3. Handlers (Packet processing)
```cpp
// Input processing
_input_handler.handle_player_input(_registry, _client_entity_ids, client_id, payload);

// Powerup processing
_powerup_handler.handle_powerup_choice(_registry, _client_entity_ids, 
                                        _players_who_chose_powerup, client_id, choice);

// Weapon processing
bool all_ready = _weapon_handler.handle_weapon_upgrade_choice(_registry, _client_entity_ids, 
                                                               client_id, upgrade_choice);
```

### 4. Managers de Jeu
```cpp
// PlayerManager - Player management
_player_manager.create_player(_registry, _client_entity_ids, client_id, x, y);
_player_manager.remove_player(_registry, _client_entity_ids, client_id);
_player_manager.check_all_players_dead(_registry, _client_entity_ids);

// LevelManager - Level management
_level_manager.advance_level(_registry);
_level_manager.clear_enemies_and_projectiles(_registry, _boss_entity);

// BossManager - Boss AI
_boss_manager.spawn_boss_level_5(_registry, _boss_entity, ...);
_boss_manager.update_boss_behavior(_registry, _boss_entity, _client_entity_ids, ...);
```

---

##  Design Patterns Utilisés

| Pattern | Où | Pourquoi |
|---------|-----|----------|
| **Facade** | GameSession | Component orchestration |
| **Strategy** | Broadcasters, Handlers | Different processing strategies |
| **Single Responsibility** | Tous les composants | One class = one responsibility |
| **Dependency Injection** | Managers | Registry passed by reference |

---

##  Data Flow

```
                    ┌─────────────────┐
                    │   UDPServer     │
                    │   (network)      │
                    └────────┬────────┘
                             │ packets
                             ▼
┌─────────────────────────────────────────────────────────┐
│                     GameSession                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐   │
│  │ InputHandler │  │PowerupHandler│  │WeaponHandler │   │
│  └──────────────┘  └──────────────┘  └──────────────┘   │
│                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐   │
│  │PlayerManager │  │ LevelManager │  │ BossManager  │   │
│  └──────────────┘  └──────────────┘  └──────────────┘   │
│                                                          │
│  ┌──────────────────────────────────────────────────┐   │
│  │              Broadcasters (sending)                 │   │
│  │  Entity | Lobby | Game | Powerup                  │   │
│  └──────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

---

##  Points d'Attention

1. **Namespace mandatory:**
   ```cpp
   using namespace server;  // or prefix with server::
   ```

2. **GameSession replaces Game:**
   ```cpp
   // Avant
   Game gameInstance;
   
   // Après
   server::GameSession gameInstance;
   ```

3. **Registry passed by reference:**
   Managers/handlers receive `registry&` as parameter, they don't own it.

---

##  Testé et Fonctionnel
- Build: 
- Tests: 4/4 passent
- Multijoueur: Works as before
