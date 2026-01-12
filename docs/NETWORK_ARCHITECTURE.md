# 🌐 Architecture Réseau R-TYPE

## 📋 Vue d'Ensemble

Ce document détaille l'architecture réseau du projet R-TYPE, incluant :
- **Clock Synchronization** - Timestamps client/serveur
- **Server Reconciliation** - Autorité serveur et intégration physique
- **Input Delaying** - Buffer d'inputs pour synchronisation multi-joueurs
- **Packet Reliability** - Fiabilité UDP avec ACK/retry
- **Client Prediction** - Prédiction et correction côté client
- **Position History** - Historique pour replay et serpent boss

---

## ⏰ Clock Synchronization

### ✅ Implémentation Actuelle

**1. Timestamps Client**
```cpp
// client/src/network/NetworkClient.cpp
auto now = std::chrono::steady_clock::now();
auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_);
uint32_t timestamp = static_cast<uint32_t>(elapsed.count());
```
- Chaque client maintient un `start_time_` (début de connexion)
- Les inputs sont horodatés avec timestamp en millisecondes
- Utilise `std::chrono::steady_clock` (monotone, pas de saut système)

**2. Réception Côté Serveur**
```cpp
// server/src/handlers/InputHandler.cpp
RType::BinarySerializer deserializer(data);
uint8_t input_mask;
uint32_t timestamp;
deserializer >> input_mask >> timestamp;

// Stockage dans InputBuffer
struct InputEntry {
    uint32_t client_timestamp;  // ✅ Timestamp client préservé
    uint8_t input_mask;
    std::chrono::steady_clock::time_point receive_time;  // ✅ Timestamp serveur
};
```

**3. Usage Actuel**
- `client_timestamp` : Timestamp client envoyé avec chaque input
- `receive_time` : Timestamp serveur utilisé pour input delaying (50ms)

---

## 🔄 Server Reconciliation

### ✅ Architecture Actuelle

**1. Autorité Serveur**
```cpp
// Flux correct
Client Input → InputBuffer (50ms) → apply_input_to_player() → movementSystem() → Broadcast
```
- Le serveur est **autoritaire** : calcule toutes les positions
- Les clients reçoivent les positions validées par le serveur
- Pas de confiance aveugle dans les données client

**2. Bounds Enforcement**
```cpp
// game-lib/src/systems/movement_system.cpp
struct bounded_movement {
    float min_x = 0.0f, max_x = 1920.0f;
    float min_y = 0.0f, max_y = 1080.0f;
};

// Clamp automatique après intégration
if (pos.x < bound.min_x) pos.x = bound.min_x;
if (pos.x > bound.max_x) pos.x = bound.max_x;
```
- Les joueurs ne peuvent **pas sortir** de la zone de jeu
- Clamp appliqué à chaque frame dans `movementSystem()`

**3. Input Buffering**
```cpp
// server/include/handlers/InputBuffer.hpp
struct InputDelayConfig {
    static constexpr int INPUT_DELAY_MS = 50;
    static constexpr size_t MAX_BUFFERED_INPUTS = 100;
};
```
- Buffer de 50ms pour synchroniser les joueurs multi-joueurs
- Évite les inputs out-of-order
- Gestion automatique du timeout (5000ms)

**4. Système de Mouvement**
```cpp
// game-lib/src/systems/movement_system.cpp
void movementSystem(registry& reg, float dt) {
    // Intégration physique : position += velocity * dt
    pos.x += vel.vx * dt;
    pos.y += vel.vy * dt;
    
    // Clamp avec bounded_movement
    if (pos.x < bound.min_x) pos.x = bound.min_x;
    if (pos.x > bound.max_x) pos.x = bound.max_x;
}
```
- Intégration Euler pour la physique
- Application des contraintes de bounds après intégration

---

## � Input Delaying System

### ✅ Implémentation Complète

**Documentation :** `docs/INPUT_DELAYING_IMPLEMENTATION.md`

**Configuration :**
```cpp
struct InputDelayConfig {
    static constexpr int INPUT_DELAY_MS = 50;          // Délai avant application
    static constexpr size_t MAX_BUFFERED_INPUTS = 100; // Capacité buffer
    static constexpr int INPUT_TIMEOUT_MS = 5000;      // Expiration
};
```

**Flux :**
```
Client send_input(timestamp, mask)
    ↓
Server receive → InputBuffer::add_input()
    ↓ [Wait 50ms]
InputBuffer::get_ready_inputs()
    ↓
apply_input_to_player() → set velocity
    ↓
movementSystem() → integrate position
    ↓
EntityBroadcaster → send to all clients
```

**Tests :** `tests/network/test_input_buffer.cpp` (25+ tests)

---

## 📦 Packet Reliability System

### ✅ Implémentation Complète

**Documentation :** `docs/RELIABLE_UDP_IMPLEMENTATION.md`

**Configuration :**
```cpp
struct ReliabilityConfig {
    static constexpr int MAX_RETRIES = 3;              // Tentatives
    static constexpr int RETRY_TIMEOUT_MS = 200;       // Délai retry
    static constexpr size_t REORDER_WINDOW_SIZE = 64;  // Fenêtre reordering
    static constexpr int REORDER_BUFFER_TIMEOUT_MS = 500; // Timeout buffer
    static constexpr size_t DUPLICATE_CACHE_SIZE = 256;   // Cache duplicatas
    static constexpr int DUPLICATE_CACHE_TTL_MS = 5000;   // TTL cache
};
```

**Fonctionnalités :**
- ✅ ACK/NACK avec retry automatique
- ✅ Reordering buffer (64 paquets, 500ms)
- ✅ Détection de duplicatas (cache 256 entrées)
- ✅ Sequence IDs (uint32_t)
- ✅ Cleanup automatique des états expirés

**Tests :** `tests/network/test_packet_reliability.cpp` (45+ tests)

---

## 🎮 Client-Side Prediction

### ✅ Implémentation Complète

**Documentation :** `docs/ROLLBACK_REPLAY_SYSTEM.md`

**Composants :**

**1. Client-Side Prediction**
```cpp
// client/src/game/Game.cpp
float predicted_player_x_, predicted_player_y_;  // Position prédite
float correction_speed_ = 10.0f;                 // Vitesse correction
float snap_threshold_ = 50.0f;                   // Seuil snap

// Smooth correction si erreur < 50px
// Snap instantané si erreur >= 50px
```

**2. Position History (Serpent Boss)**
```cpp
// game/include/components/logic_components.hpp
struct position_history {
    static constexpr size_t MAX_HISTORY = 60;  // 1 seconde à 60 FPS
    std::array<PositionSnapshot, MAX_HISTORY> history;
    size_t head = 0;
    
    // Permet delay de segments du serpent (follow-the-leader)
};
```

**3. Packet Reordering**
```cpp
// server/include/network/PacketReliability.hpp
struct ClientReliabilityState {
    std::map<uint32_t, BufferedPacket> reorder_buffer;
    uint32_t expected_recv_sequence = 1;
    
    // Rejoue automatiquement les paquets bufferisés dans l'ordre
};
```

**Tests :** 
- `tests/game/test_client_prediction.cpp` (50+ tests)
- `tests/game/test_position_history.cpp` (55+ tests)

---

## � Fichiers de l'Architecture

### Client
- `client/include/network/NetworkClient.hpp` - Gestion connexion, timestamps
- `client/src/network/NetworkClient.cpp` - send_input(), decode_entities()
- `client/include/game/Game.hpp` - Prédiction client (predicted_player_x/y)
- `client/src/game/Game.cpp` - Correction smooth/snap

### Serveur
- `server/include/network/UDPServer.hpp` - Serveur UDP principal
- `server/include/network/PacketReliability.hpp` - Système fiabilité
- `server/include/handlers/InputBuffer.hpp` - Buffer inputs (50ms delay)
- `server/include/handlers/InputHandler.hpp` - Application des inputs
- `server/src/handlers/InputHandler.cpp` - Logique mouvement/tir
- `server/src/game/ServerCore.cpp` - Boucle principale serveur
- `server/src/game/GameSession.cpp` - Session de jeu, update loop

### Game Logic
- `game-lib/src/systems/movement_system.cpp` - Intégration physique
- `game-lib/include/components/logic_components.hpp` - Composants ECS
- `game-lib/include/entities/player_factory.cpp` - Création joueurs

### Tests
- `tests/network/test_input_buffer.cpp` - Tests input delaying (25+)
- `tests/network/test_packet_reliability.cpp` - Tests fiabilité UDP (45+)
- `tests/game/test_client_prediction.cpp` - Tests prédiction (50+)
- `tests/game/test_position_history.cpp` - Tests historique (55+)

---

## � Résumé des Systèmes

| Système | Status | Tests | Documentation |
|---------|--------|-------|---------------|
| **Clock Timestamps** | ✅ Implémenté | - | Ce doc |
| **Server Authority** | ✅ Implémenté | - | Ce doc |
| **Input Delaying** | ✅ Implémenté | 25+ tests | `INPUT_DELAYING_IMPLEMENTATION.md` |
| **Packet Reliability** | ✅ Implémenté | 45+ tests | `RELIABLE_UDP_IMPLEMENTATION.md` |
| **Client Prediction** | ✅ Implémenté | 50+ tests | `ROLLBACK_REPLAY_SYSTEM.md` |
| **Position History** | ✅ Implémenté | 55+ tests | `ROLLBACK_REPLAY_SYSTEM.md` |
| **Bounded Movement** | ✅ Implémenté | - | Ce doc |

**Total Tests Réseau/Jeu : 170+ tests**

---

## 🎯 Caractéristiques Principales

### ✅ Ce Qui Fonctionne

1. **Autorité Serveur Complète**
   - Toutes les positions calculées côté serveur
   - Clients reçoivent positions validées
   - Bounded movement automatique

2. **Fiabilité UDP Robuste**
   - ACK/retry (3 tentatives, 200ms timeout)
   - Reordering buffer (64 paquets)
   - Détection duplicatas (cache 256 entrées)

3. **Input Synchronization**
   - Buffer de 50ms côté serveur
   - Progressive release des inputs
   - Timeout automatique (5000ms)

4. **Client Experience**
   - Prédiction instantanée (0ms lag ressenti)
   - Correction smooth (< 50px erreur)
   - Snap pour grandes divergences (>= 50px)

5. **Serpent Boss Mechanics**
   - Historique 60 frames (1 sec à 60 FPS)
   - Segments suivent la tête avec délai
   - Système follow-the-leader

### 📈 Performances

- **Tick Rate Serveur :** 60 FPS (16.67ms/frame)
- **Input Delay :** 50ms (3 frames à 60 FPS)
- **Packet Retry :** 200ms timeout, 3 tentatives max
- **Reorder Window :** 64 paquets, 500ms buffer
- **Client Prediction :** 0ms lag ressenti
- **Correction Speed :** 10.0 units/frame (smooth)

---

## 📚 Documentation Associée

- **`INPUT_DELAYING_IMPLEMENTATION.md`** (600+ lignes)
  - Architecture complète du système de buffering
  - Configuration et tuning
  - Guide de test et debug

- **`RELIABLE_UDP_IMPLEMENTATION.md`** (600+ lignes)
  - Protocole ACK/retry détaillé
  - Gestion reordering et duplicatas
  - Scénarios de test

- **`ROLLBACK_REPLAY_SYSTEM.md`** (900+ lignes)
  - Client-side prediction expliquée
  - Position history pour serpent
  - Packet reordering côté client

- **`RUNNING_TESTS.md`**
  - Guide d'exécution des 170+ tests
  - Commandes CTest
  - Filtres et debugging

---

**Document créé le :** 12 janvier 2026  
**Auteur :** Documentation Architecture R-TYPE  
**Version :** 2.0 - Implémentation Actuelle  
**Statut :** ✅ Systèmes Opérationnels
