# Architecture Modulaire R-TYPE - Couche Engine

## 📋 Résumé

Ce document explique la refonte architecturale du serveur R-TYPE pour passer d'une architecture **EC** (Entity-Component) à une vraie architecture **ECS** (Entity-Component-System) modulaire avec une couche engine séparée.

## 🎯 Objectif

Répondre aux critiques du professeur :
- ✅ **Code plus modulaire** : séparation claire entre infrastructure (engine) et logique métier (game)
- ✅ **Vraie architecture ECS** : components = données pures (POD), systems = logique pure
- ✅ **Couche engine claire** : primitives réutilisables indépendantes du jeu

---

## 🏗️ Structure de l'Architecture

```
R-TYPE/
├── engine/                    ← COUCHE ENGINE (Infrastructure)
│   ├── core/
│   │   ├── ISystem.hpp        ← Interface pure pour systèmes
│   │   ├── SystemManager.*    ← Gestionnaire de cycle de vie des systèmes
│   │   └── GameEngine.*       ← Orchestrateur principal (Registry + Systems)
│   └── ecs/
│       ├── registry.hpp       ← Stockage entités/composants
│       ├── entity.hpp
│       ├── components.hpp     ← Composants de base (POD)
│       └── ...
│
├── game-lib/                  ← COUCHE GAME (Logique métier)
│   ├── include/
│   │   ├── systems/
│   │   │   ├── system_wrappers.hpp  ← Wrappers ISystem pour systèmes existants
│   │   │   ├── shooting_system.hpp
│   │   │   ├── movement_system.hpp
│   │   │   └── ...
│   │   └── components/
│   │       └── game_components.hpp   ← Composants spécifiques au jeu
│   └── src/
│
└── server/                    ← APPLICATION (Serveur de jeu)
    ├── include/
    │   ├── game/
    │   │   └── GameSession.hpp       ← Utilise GameEngine
    │   └── network/
    │       ├── NetworkDispatcher.hpp ← Parsing réseau → Commands
    │       └── INetworkCommand.hpp   ← Interface commandes réseau
    └── src/
```

---

## 🔑 Principes de l'Architecture ECS Modulaire

### 1. **Components = Données uniquement (POD)**

❌ **Avant (EC)** :
```cpp
struct weapon {
    int damage;
    float cooldown;
    void shoot() { /* logique ici */ }  // ❌ Logique dans le composant
};
```

✅ **Après (ECS)** :
```cpp
struct weapon {
    int damage;
    float cooldown;
    float time_since_last_shot;
    // Pas de méthodes, juste des données
};
```

### 2. **Systems = Logique pure**

Les systèmes itèrent sur les composants via le registry et appliquent la logique.

```cpp
class ShootingSystem : public engine::ISystem {
public:
    void update(registry& reg, float dt) override {
        // Itère sur toutes les entités avec weapon + position
        auto& weapons = reg.get_components<weapon>();
        auto& positions = reg.get_components<position>();
        
        for (size_t i = 0; i < weapons.size(); ++i) {
            if (weapons[i].has_value() && positions[i].has_value()) {
                // Logique de tir ici
            }
        }
    }
};
```

### 3. **Engine Layer = Infrastructure réutilisable**

La couche engine fournit des **primitives génériques** :

#### `ISystem` : Interface pour tout système
```cpp
class ISystem {
public:
    virtual void init(registry& reg) = 0;      // Initialisation
    virtual void update(registry& reg, float dt) = 0;  // Update frame
    virtual void shutdown(registry& reg) = 0;  // Nettoyage
};
```

#### `SystemManager` : Gestion du cycle de vie
```cpp
SystemManager mgr;
mgr.register_system(std::make_unique<MovementSystem>());
mgr.register_system(std::make_unique<CollisionSystem>());

mgr.init_all(registry);         // Initialise tous les systèmes
mgr.update_all(registry, dt);   // Update dans l'ordre d'enregistrement
mgr.shutdown_all(registry);     // Shutdown propre
```

#### `GameEngine` : Orchestrateur central
```cpp
GameEngine engine;

// Enregistrement des systèmes
engine.register_system(std::make_unique<ShootingSystem>());
engine.register_system(std::make_unique<MovementSystem>());
engine.register_system(std::make_unique<CollisionSystem>());

// Cycle de vie
engine.init();
while (running) {
    engine.update(dt);  // Exécute tous les systèmes
}
engine.shutdown();
```

---

## 🔄 Avant / Après : GameSession

### ❌ Avant (Code monolithique)

```cpp
class GameSession {
private:
    registry _registry;  // Registry directement exposé
    
    void update_game_state(float dt) {
        // Appels directs aux systèmes (couplage fort)
        shootingSystem(_registry, dt);
        enemyShootingSystem(_registry, dt);
        movementSystem(_registry, dt);
        collisionSystem(_registry);
        cleanupSystem(_registry);
    }
};
```

**Problèmes** :
- Logique dispersée (difficile à tester)
- Couplage fort entre GameSession et systèmes
- Ordre d'exécution implicite
- Impossible de réutiliser la loop ailleurs

### ✅ Après (Architecture modulaire)

```cpp
class GameSession {
private:
    engine::GameEngine _engine;  // Délègue à l'engine
    
    void update_game_state(float dt) {
        // L'engine gère l'exécution de tous les systèmes
        _engine.update(dt);
        
        // Logique spécifique au serveur (boss, powerups, etc.)
        _boss_manager.update(...);
    }
};

// Constructeur : enregistrement des systèmes
GameSession::GameSession() {
    auto& reg = _engine.get_registry();
    
    // Enregistrement composants
    reg.register_component<position>();
    reg.register_component<velocity>();
    // ...
    
    // Enregistrement systèmes (ordre explicite)
    _engine.register_system(std::make_unique<ShootingSystem>());
    _engine.register_system(std::make_unique<EnemyShootingSystem>());
    _engine.register_system(std::make_unique<WaveSystem>());
    _engine.register_system(std::make_unique<MovementSystem>());
    _engine.register_system(std::make_unique<CollisionSystem>());
    _engine.register_system(std::make_unique<CleanupSystem>());
    
    _engine.init();
}
```

**Avantages** :
- ✅ Séparation claire engine/game
- ✅ Systèmes testables indépendamment
- ✅ Ordre d'exécution explicite
- ✅ Engine réutilisable pour d'autres projets

---

## 🌐 Séparation Réseau / Logique (NetworkDispatcher)

### ❌ Avant : Parsing réseau mélangé à la logique

```cpp
void process_network_events(UDPServer& server) {
    NetworkPacket packet;
    while (server.get_input_packet(packet)) {
        // Parsing
        deserializer >> opcode;
        
        // Logique métier directement ici ❌
        switch (opcode) {
            case Input:
                _player_manager.create_player(...);
                _input_handler.handle_input(...);
                break;
        }
    }
}
```

### ✅ Après : NetworkDispatcher (Command Pattern)

```cpp
// 1. Dispatcher parse et crée des commandes
class NetworkDispatcher {
    std::queue<std::unique_ptr<INetworkCommand>> poll_commands(UDPServer& server);
};

// 2. Commandes encapsulent les actions
class INetworkCommand {
    virtual void execute(registry& reg, ...) = 0;
};

// 3. GameSession exécute les commandes
void process_network_events(UDPServer& server) {
    auto commands = _dispatcher.poll_commands(server);
    while (!commands.empty()) {
        commands.front()->execute(_engine.get_registry(), ...);
        commands.pop();
    }
}
```

**Avantages** :
- ✅ Parsing réseau séparé de la logique
- ✅ Testable avec mocks
- ✅ Commandes rejouables / loggables

---

## 📊 Hiérarchie des Responsabilités

```
┌─────────────────────────────────────────────────┐
│          APPLICATION (GameSession)              │
│  • Gère états de jeu (lobby, levels, boss)     │
│  • Coordonne engine + network + managers       │
└─────────────────┬───────────────────────────────┘
                  │
        ┌─────────┴──────────┐
        │                    │
┌───────▼────────┐  ┌────────▼──────────┐
│  GAME LAYER    │  │  ENGINE LAYER     │
│  (game-lib)    │  │  (engine/)        │
│                │  │                   │
│ • Systems      │  │ • ISystem         │
│ • Components   │  │ • SystemManager   │
│ • Factories    │  │ • GameEngine      │
│ • Handlers     │  │ • Registry (ECS)  │
└────────────────┘  └───────────────────┘
```

**Engine Layer** : Infrastructure réutilisable, indépendante du jeu  
**Game Layer** : Logique métier (systèmes de tir, ennemis, boss)  
**Application** : Orchestration haut niveau (lobby, network, niveaux)

---

## ✅ Checklist de Modularité

- [x] **Components = POD** : pas de logique dans les structs
- [x] **Systems = classes ISystem** : logique isolée et testable
- [x] **Engine indépendant** : peut être réutilisé pour autre jeu
- [x] **Ordre d'exécution explicite** : via register_system()
- [x] **Network découplé** : parsing → commands → execute
- [x] **Tests possibles** : mock registry, mock systems

---

## 🚀 Prochaines Améliorations (Optionnelles)

1. **Extraire boss logic en BossSystem** (actuellement dans GameSession)
2. **Command pattern complet** : implémenter toutes les commandes réseau
3. **Event bus** : pour communication inter-systèmes sans couplage
4. **Resource manager** : gérer assets, config centralisés
5. **Tests unitaires** : tester chaque système isolément

---

## 📝 Résumé pour le Prof

**Avant** : EC (Entity-Component) avec logique dispersée, couplage fort  
**Après** : ECS (Entity-Component-System) modulaire avec couche engine claire

**Points clés** :
- ✅ Séparation infrastructure (engine) / logique métier (game)
- ✅ Components purs (données) + Systems purs (logique)
- ✅ Architecture testable, extensible, réutilisable
- ✅ NetworkDispatcher sépare parsing réseau de la logique
- ✅ GameEngine orchestre le tout de façon modulaire

**Fichiers créés** :
- `engine/core/ISystem.hpp`
- `engine/core/SystemManager.{hpp,cpp}`
- `engine/core/GameEngine.{hpp,cpp}`
- `game-lib/include/systems/system_wrappers.hpp`
- `server/include/network/NetworkDispatcher.hpp`
- `server/include/network/INetworkCommand.hpp`

**Fichiers modifiés** :
- `server/include/game/GameSession.hpp` (utilise GameEngine)
- `server/src/game/GameSession.cpp` (enregistre systèmes, appelle engine.update())
- `engine/CMakeLists.txt` (bibliothèque STATIC avec .cpp)
