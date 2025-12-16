# 🖥️ Refactoring Server R-TYPE

## Résumé
Le serveur a été refactoré pour séparer les responsabilités et réduire le "God Object" `Game.cpp` de **1111 lignes → 475 lignes**.

---

## 📁 Nouvelle Structure

```
server/
├── include/
│   ├── common/           ← Types et constantes partagés
│   │   ├── ClientEndpoint.hpp
│   │   ├── GameConstants.hpp
│   │   ├── InputKey.hpp
│   │   ├── NetworkPacket.hpp
│   │   └── SafeQueue.hpp
│   ├── game/             ← Logique métier
│   │   ├── BossManager.hpp
│   │   ├── GameSession.hpp
│   │   ├── LevelManager.hpp
│   │   └── PlayerManager.hpp
│   ├── handlers/         ← Traitement des inputs clients
│   │   ├── InputHandler.hpp
│   │   ├── PowerupHandler.hpp
│   │   └── WeaponHandler.hpp
│   └── network/          ← Couche réseau
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

## 🔧 Changements Principaux

### 1. Namespace `server::`
Toutes les classes sont dans le namespace `server::`:
```cpp
server::GameSession gameInstance;
server::UDPServer server(io_context, bind_address, port);
```

### 2. Broadcasters (Envoi réseau séparé)
```cpp
// Avant: broadcast_entity_positions() dans Game.cpp
// Après: classes dédiées
_entity_broadcaster.broadcast_entity_positions(server, _registry, _client_entity_ids);
_lobby_broadcaster.broadcast_lobby_status(server, _client_ready_status);
_game_broadcaster.broadcast_game_over(server);
_powerup_broadcaster.broadcast_powerup_status(server, _registry, _client_entity_ids);
```

**Broadcasters créés:**
- `EntityBroadcaster` - Positions des entités
- `LobbyBroadcaster` - Statut du lobby
- `GameBroadcaster` - Level info, game over
- `PowerupBroadcaster` - Statut des powerups

### 3. Handlers (Traitement des packets)
```cpp
// Traitement input
_input_handler.handle_player_input(_registry, _client_entity_ids, client_id, payload);

// Traitement powerups
_powerup_handler.handle_powerup_choice(_registry, _client_entity_ids, 
                                        _players_who_chose_powerup, client_id, choice);

// Traitement armes
bool all_ready = _weapon_handler.handle_weapon_upgrade_choice(_registry, _client_entity_ids, 
                                                               client_id, upgrade_choice);
```

### 4. Managers de Jeu
```cpp
// PlayerManager - Gestion des joueurs
_player_manager.create_player(_registry, _client_entity_ids, client_id, x, y);
_player_manager.remove_player(_registry, _client_entity_ids, client_id);
_player_manager.check_all_players_dead(_registry, _client_entity_ids);

// LevelManager - Gestion des niveaux
_level_manager.advance_level(_registry);
_level_manager.clear_enemies_and_projectiles(_registry, _boss_entity);

// BossManager - IA du boss
_boss_manager.spawn_boss_level_5(_registry, _boss_entity, ...);
_boss_manager.update_boss_behavior(_registry, _boss_entity, _client_entity_ids, ...);
```

---

## 📊 Design Patterns Utilisés

| Pattern | Où | Pourquoi |
|---------|-----|----------|
| **Facade** | GameSession | Orchestration des composants |
| **Strategy** | Broadcasters, Handlers | Différentes stratégies de traitement |
| **Single Responsibility** | Tous les composants | Une classe = une responsabilité |
| **Dependency Injection** | Managers | Passage du registry par référence |

---

## 🔄 Flux de Données

```
                    ┌─────────────────┐
                    │   UDPServer     │
                    │   (réseau)      │
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
│  │              Broadcasters (envoi)                 │   │
│  │  Entity | Lobby | Game | Powerup                  │   │
│  └──────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

---

## ⚠️ Points d'Attention

1. **Namespace obligatoire:**
   ```cpp
   using namespace server;  // ou préfixer avec server::
   ```

2. **GameSession remplace Game:**
   ```cpp
   // Avant
   Game gameInstance;
   
   // Après
   server::GameSession gameInstance;
   ```

3. **Registre passé par référence:**
   Les managers/handlers reçoivent `registry&` en paramètre, ils ne le possèdent pas.

---

## ✅ Testé et Fonctionnel
- Build: ✅
- Tests: 4/4 passent
- Multijoueur: Fonctionne comme avant
