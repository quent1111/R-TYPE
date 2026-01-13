# 🚀 Implémentation UDP Fiable - Gestion de Perte, Reordering et Duplication

## 📋 Vue d'Ensemble

Ce document décrit l'implémentation complète d'un système UDP fiable avec gestion de :
- **Perte de paquets** (Packet Loss) via ACK + Retransmission ✅ **IMPLÉMENTÉ**
- **Réordonnancement** (Reordering) via fenêtre glissante + buffer ✅ **IMPLÉMENTÉ**
- **Duplication** via cache de sequence IDs ✅ **IMPLÉMENTÉ**

## ✅ État d'Implémentation

| Composant | Statut | Fichiers |
|-----------|--------|----------|
| Structure de fiabilité | ✅ Terminé | `server/include/network/PacketReliability.hpp` |
| Intégration serveur | ✅ Terminé | `server/include/network/UDPServer.hpp` |
| Méthode send_reliable() | ✅ Terminé | `server/src/network/UDPServer.cpp:338-384` |
| Méthode handle_ack() | ✅ Terminé | `server/src/network/UDPServer.cpp:405-424` |
| Méthode retry_unacked_packets() | ✅ Terminé | `server/src/network/UDPServer.cpp:426-450` |
| Thread de retry | ✅ Terminé | `server/src/network/UDPServer.cpp:452-459` |
| Détection ACK dans handle_receive() | ✅ Terminé | `server/src/network/UDPServer.cpp:120-172` |
| Nettoyage déconnexion | ✅ Terminé | `server/src/network/UDPServer.cpp:461-471` |
| Démarrage thread | ✅ Terminé | `UDPServer constructor/destructor` |
| OpCode ACK (0x60) | ✅ Terminé | `src/Common/Opcodes.hpp` |
| Détection opcodes fiables | ✅ Terminé | `handle_receive() - opcodes 0x02,0x22,0x30,0x50,0x40,0x37` |

## 🎯 Architecture

### Stratégie Hybride (IMPLÉMENTÉE)

```
Messages Fréquents (unreliable)     Messages Critiques (reliable)
─────────────────────────────       ────────────────────────────
• EntityPosition (0x13)             • LoginAck (0x02)        ✅
• Input (0x10)                      • StartGame (0x22)       ✅
• PowerUpStatus (0x36)              • LevelStart (0x30)      ✅
• LobbyStatus (0x21)                • BossSpawn (0x50)       ✅
• ListLobbies (0x23)                • GameOver (0x40)        ✅
                                    • PowerUpCards (0x37)    ✅
UDP classique
Pas de garantie                     UDP + Fiabilité
Traitement direct                   ACK + Retry + Séquençage
                                    Reordering + Anti-duplication
```

**Note importante** : La détection des opcodes fiables se fait dans `handle_receive()` ligne 133-139. Seuls ces 6 opcodes utilisent le système de fiabilité.

## 🔧 Composants Principaux

### 1. **ClientReliabilityState** (`PacketReliability.hpp`)

État de fiabilité maintenu pour chaque client connecté.

```cpp
struct ClientReliabilityState {
    // ÉMISSION (gestion perte)
    uint32_t next_send_sequence;        // Prochain sequence ID à envoyer
    deque<PendingPacket> pending_acks;  // Paquets en attente d'ACK
    
    // RÉCEPTION (gestion reordering)
    uint32_t expected_recv_sequence;    // Prochain sequence ID attendu
    map<uint32_t, BufferedPacket> reorder_buffer; // Buffer réordonnancement
    
    // ANTI-DUPLICATION
    set<uint32_t> duplicate_cache;      // Cache des IDs déjà vus
    map<uint32_t, DuplicateCacheEntry> cache_timestamps; // TTL
};
```

### 2. **Configuration** (`ReliabilityConfig`)

```cpp
static constexpr int MAX_RETRIES = 3;              // 3 tentatives max
static constexpr int RETRY_TIMEOUT_MS = 200;      // 200ms entre retries
static constexpr uint32_t REORDER_WINDOW_SIZE = 64; // Fenêtre 64 paquets
static constexpr int REORDER_BUFFER_TIMEOUT_MS = 500; // Timeout buffer 500ms
static constexpr uint32_t DUPLICATE_CACHE_SIZE = 256; // Cache 256 entrées
static constexpr int DUPLICATE_CACHE_TTL_MS = 5000;  // TTL 5 secondes
```

## 📦 Format des Paquets

### Paquet Normal (unreliable)
```
[Magic:2B][OpCode:1B][Payload:variable]
```

### Paquet Fiable (reliable)
```
[Magic:2B][OpCode:1B][SequenceID:4B][Payload:variable]
```

### Paquet ACK
```
[Magic:2B][OpCode=0x60:1B][SequenceID:4B]
```

## 🔄 Flux de Fonctionnement

### Émission (Serveur → Client)

```
1. send_reliable(client_id, opcode, payload)
   ├─ Obtenir sequence_id = state.get_next_send_sequence()
   ├─ Construire paquet: [Magic][OpCode][SeqID][Payload]
   ├─ Compresser si nécessaire
   ├─ Envoyer via UDP
   └─ Stocker dans pending_acks pour retry
   
2. retry_unacked_packets() (thread séparé, boucle 50ms)
   ├─ Pour chaque client:
   │  └─ Pour chaque pending_ack:
   │     ├─ Si timeout écoulé (200ms):
   │     │  ├─ Si retry_count < 3:
   │     │  │  └─ Renvoyer le paquet
   │     │  └─ Sinon:
   │     │     └─ Log warning + supprimer
   │     └─ mark_resent()
   
3. handle_ack(client_id, sequence_id)
   └─ Supprimer de pending_acks → Succès !
```

### Réception (Client → Serveur)

```
1. handle_receive(data)
   ├─ Décompresser
   ├─ Extraire sequence_id (si présent)
   ├─ ready_packets = state.process_received_packet(seq_id, data)
   │  ├─ is_duplicate(seq_id) ?
   │  │  └─ OUI → Ignorer (log debug)
   │  ├─ is_in_reorder_window(seq_id) ?
   │  │  └─ NON → Ignorer (trop ancien ou trop en avance)
   │  ├─ seq_id == expected_recv_sequence ?
   │  │  ├─ OUI → Traiter immédiatement
   │  │  │  ├─ Ajouter à ready_packets
   │  │  │  ├─ expected_recv_sequence++
   │  │  │  └─ Vider reorder_buffer des paquets consécutifs
   │  │  └─ NON → Buffer pour réordonnancement
   │  │     └─ reorder_buffer[seq_id] = packet
   │  └─ cleanup_reorder_buffer() (supprimer expirés)
   │
   ├─ Pour chaque packet dans ready_packets:
   │  └─ Traiter normalement (dispatch selon OpCode)
   │
   └─ Envoyer ACK au serveur
      └─ send_ack(sequence_id)
```

## 🛡️ Gestion de la Perte de Paquets

### Mécanisme ACK + Retransmission

**Scénario : Paquet perdu**

```
Temps   Serveur                          Client
─────   ─────────────────────────────   ──────────────────────
t0      send_reliable(LoginAck, seq=1)
        │ Envoi UDP
        │ pending_acks.push(seq=1)
        └─────────────X (PERDU)         
        
t200    retry_unacked_packets()
        │ timeout écoulé !
        │ retry_count=0 → Renvoyer
        └─────────────────────────────→ Reçu !
                                         │ Traiter LoginAck
                                         └─ send_ack(seq=1)
        
t220    handle_ack(seq=1)
        └─ pending_acks.erase(seq=1)    ✓ Succès !
```

**Scénario : ACK perdu (mais paquet reçu)**

```
Temps   Serveur                          Client
─────   ─────────────────────────────   ──────────────────────
t0      send_reliable(LoginAck, seq=1)
        │ pending_acks.push(seq=1)
        └─────────────────────────────→ Reçu !
                                         │ Traiter LoginAck
                                         └─X ACK perdu
        
t200    retry_unacked_packets()
        │ Pas d'ACK reçu, renvoyer
        └─────────────────────────────→ Reçu (doublon)
                                         │ is_duplicate(seq=1) = TRUE
                                         │ Ignorer paquet
                                         └─ send_ack(seq=1)
        
t220    handle_ack(seq=1)
        └─ pending_acks.erase(seq=1)    ✓ Succès !
```

## 🔀 Gestion du Reordering

### Fenêtre Glissante + Buffer

**Scénario : Paquets arrivent dans le désordre**

```
Ordre envoi : seq=1, seq=2, seq=3, seq=4
Ordre reçu  : seq=1, seq=3, seq=4, seq=2

État Réception:
─────────────────────────────────────────────────────────────

Reçoit seq=1 (expected=1)
├─ expected=1, seq=1 → Match !
├─ Traiter immédiatement
└─ expected_recv_sequence = 2

Reçoit seq=3 (expected=2)
├─ expected=2, seq=3 → Hors ordre !
├─ is_in_reorder_window(3) ? OUI (dans [2, 66))
├─ Bufferiser: reorder_buffer[3] = packet
└─ expected_recv_sequence = 2 (inchangé)

Reçoit seq=4 (expected=2)
├─ expected=2, seq=4 → Hors ordre !
├─ is_in_reorder_window(4) ? OUI (dans [2, 66))
├─ Bufferiser: reorder_buffer[4] = packet
└─ expected_recv_sequence = 2 (inchangé)

Reçoit seq=2 (expected=2)
├─ expected=2, seq=2 → Match !
├─ Traiter immédiatement
├─ expected_recv_sequence = 3
├─ Vérifier buffer: reorder_buffer[3] existe !
│  ├─ Traiter seq=3
│  ├─ expected_recv_sequence = 4
│  └─ Vérifier buffer: reorder_buffer[4] existe !
│     ├─ Traiter seq=4
│     └─ expected_recv_sequence = 5
└─ Tous les paquets traités dans l'ordre ! ✓
```

**Fenêtre de Réordonnancement**

```
expected_recv_sequence = 10
REORDER_WINDOW_SIZE = 64

Fenêtre acceptée : [10, 74)

seq=5  → Rejeté (trop ancien, < 10)
seq=10 → Accepté (attendu)
seq=15 → Accepté (bufferisé)
seq=73 → Accepté (limite haute)
seq=74 → Rejeté (hors fenêtre, >= 74)
```

## 🔁 Gestion de la Duplication

### Cache de Sequence IDs avec TTL

**Mécanisme**

```cpp
bool is_duplicate(uint32_t seq_id) {
    cleanup_duplicate_cache(); // Nettoyer entrées expirées
    
    if (duplicate_cache.find(seq_id) != duplicate_cache.end()) {
        return true; // DUPLICATA !
    }
    
    duplicate_cache.insert(seq_id);
    cache_timestamps[seq_id] = now();
    
    // Limiter taille cache à 256 entrées
    if (duplicate_cache.size() > 256) {
        erase_oldest();
    }
    
    return false;
}
```

**Scénario : Paquet dupliqué par le réseau**

```
Temps   Réception                     État Cache
─────   ──────────────────────────   ──────────────────────
t0      Reçoit seq=5
        │ is_duplicate(5) ? NON
        │ cache.insert(5)
        │ Traiter paquet              cache={5}
        
t50     Reçoit seq=5 (doublon réseau)
        │ is_duplicate(5) ? OUI !
        └─ Ignorer paquet             cache={5}
        
t5000   cleanup_duplicate_cache()
        └─ seq=5 expiré (TTL 5s)      cache={}
```

## 🔌 Intégration dans UDPServer (✅ IMPLÉMENTÉ)

### État Actuel de l'Implémentation

**Fichiers modifiés :**
- `server/include/network/PacketReliability.hpp` - Structure complète (218 lignes)
- `server/include/network/UDPServer.hpp` - Membres et déclarations ajoutés
- `server/src/network/UDPServer.cpp` - Toutes les méthodes implémentées

### 1. Membres ajoutés dans UDPServer.hpp ✅

```cpp
// Ligne 7 : Include
#include "network/PacketReliability.hpp"

// Ligne 20 : Include thread
#include <thread>

// Lignes 37-40 : Membres privés
std::map<int, RType::ClientReliabilityState> client_reliability_;
std::mutex reliability_mutex_;
std::thread retry_thread_;

// Lignes 57-63 : Méthodes publiques
void send_reliable(int client_id, uint8_t opcode, const std::vector<uint8_t>& payload);
void send_ack(int client_id, uint32_t sequence_id);
void handle_ack(int client_id, uint32_t sequence_id);
void retry_unacked_packets();
void retry_thread_loop();
void cleanup_client_reliability(int client_id);
```

### 2. Implémentation send_reliable() ✅

**Localisation** : `server/src/network/UDPServer.cpp:338-384`

```cpp
void UDPServer::send_reliable(int client_id, uint8_t opcode, const std::vector<uint8_t>& payload) {
    std::lock_guard<std::mutex> lock(reliability_mutex_);
    
    auto& state = client_reliability_[client_id];
    uint32_t seq_id = state.get_next_send_sequence();
    
    // Construire paquet [Magic:2B][OpCode:1B][SeqID:4B][Payload]
    std::vector<uint8_t> packet;
    packet.reserve(3 + 4 + payload.size());
    
    packet.push_back(0x42); // Magic low
    packet.push_back(0xB5); // Magic high
    packet.push_back(opcode);
    
    // Sequence ID (little-endian, 4 bytes)
    packet.push_back(static_cast<uint8_t>(seq_id & 0xFF));
    packet.push_back(static_cast<uint8_t>((seq_id >> 8) & 0xFF));
    packet.push_back(static_cast<uint8_t>((seq_id >> 16) & 0xFF));
    packet.push_back(static_cast<uint8_t>((seq_id >> 24) & 0xFF));
    
    packet.insert(packet.end(), payload.begin(), payload.end());
    
    // Compression
    RType::CompressionSerializer compressor(packet);
    compressor.compress();
    
    // Envoyer immédiatement
    send_to_client(client_id, compressor.data());
    
    // Stocker pour retry
    state.pending_acks.emplace_back(seq_id, opcode, compressor.data());
    
    std::cout << "[Reliable] Sent packet seq=" << seq_id 
              << " opcode=0x" << std::hex << (int)opcode << std::dec
              << " to client " << client_id << std::endl;
}
```

**Fonctionnalités** :
- ✅ Génération séquentielle de sequence_id via `state.get_next_send_sequence()`
- ✅ Construction du paquet avec format `[Magic][OpCode][SeqID][Payload]`
- ✅ Compression automatique via `CompressionSerializer`
- ✅ Envoi immédiat via UDP
- ✅ Stockage dans `pending_acks` pour retry
- ✅ Logging détaillé

### 3. Implémentation handle_receive() modifiée ✅

**Localisation** : `server/src/network/UDPServer.cpp:93-180`

```cpp
void UDPServer::handle_receive(std::error_code ec, std::size_t bytes_received) {
    // ... décompression ...
    
    if (data.size() >= 2) {
        uint16_t magic_number = /* ... */;
        if (magic_number == 0xB542) {
            int client_id = register_client(remote_endpoint_);
            
            // 1. Détection ACK (OpCode 0x60) ✅
            if (data.size() >= 3 && data[2] == 0x60) {
                if (data.size() >= 7) {
                    uint32_t seq_id = data[3] | (data[4] << 8) | 
                                     (data[5] << 16) | (data[6] << 24);
                    handle_ack(client_id, seq_id);
                }
                // Ne pas traiter comme paquet normal
            }
            
            // 2. Détection opcodes fiables ✅
            else if (data.size() >= 7) {
                uint8_t opcode = data[2];
                bool is_reliable_opcode = (opcode == 0x02 ||  // LoginAck
                                          opcode == 0x22 ||  // StartGame
                                          opcode == 0x30 ||  // LevelStart
                                          opcode == 0x50 ||  // BossSpawn
                                          opcode == 0x40 ||  // GameOver
                                          opcode == 0x37);   // PowerUpCards
                
                if (is_reliable_opcode) {
                    // Extraction sequence_id
                    uint32_t seq_id = /* ... */;
                    std::vector<uint8_t> payload(data.begin() + 7, data.end());
                    
                    // Traitement avec fiabilité ✅
                    std::lock_guard<std::mutex> lock(reliability_mutex_);
                    auto& state = client_reliability_[client_id];
                    auto ready_packets = state.process_received_packet(seq_id, payload);
                    
                    // Envoi ACK ✅
                    send_ack(client_id, seq_id);
                    
                    // Traitement des paquets prêts (après reordering) ✅
                    for (auto& pkt : ready_packets) {
                        std::vector<uint8_t> complete_packet;
                        complete_packet.push_back(0x42);
                        complete_packet.push_back(0xB5);
                        complete_packet.push_back(opcode);
                        complete_packet.insert(complete_packet.end(), pkt.begin(), pkt.end());
                        
                        NetworkPacket packet(std::move(complete_packet), remote_endpoint_);
                        input_queue_.push(std::move(packet));
                    }
                } else {
                    // Paquet normal sans fiabilité
                    NetworkPacket packet(std::move(data), remote_endpoint_);
                    input_queue_.push(std::move(packet));
                }
            } else {
                // Paquet normal court
                NetworkPacket packet(std::move(data), remote_endpoint_);
                input_queue_.push(std::move(packet));
            }
        }
    }
}
```

**Fonctionnalités implémentées** :
- ✅ Détection des paquets ACK (OpCode 0x60)
- ✅ Extraction du sequence_id (little-endian 4 bytes)
- ✅ Appel à `handle_ack()` pour supprimer de pending_acks
- ✅ Détection des opcodes fiables (6 opcodes critiques)
- ✅ Traitement avec `state.process_received_packet()` (reordering + anti-duplication)
- ✅ Envoi automatique des ACK
- ✅ Reconstruction des paquets après reordering
- ✅ Passage direct des paquets non-fiables

### 4. Implémentation send_ack() ✅

**Localisation** : `server/src/network/UDPServer.cpp:386-403`

```cpp
void UDPServer::send_ack(int client_id, uint32_t sequence_id) {
    // Construire paquet ACK : [Magic][OpCode=0x60][SequenceID]
    std::vector<uint8_t> ack_packet;
    ack_packet.reserve(7);
    
    // Magic number
    ack_packet.push_back(0x42);
    ack_packet.push_back(0xB5);
    
    // OpCode ACK (0x60)
    ack_packet.push_back(0x60);
    
    // Sequence ID (little-endian)
    ack_packet.push_back(static_cast<uint8_t>(sequence_id & 0xFF));
    ack_packet.push_back(static_cast<uint8_t>((sequence_id >> 8) & 0xFF));
    ack_packet.push_back(static_cast<uint8_t>((sequence_id >> 16) & 0xFF));
    ack_packet.push_back(static_cast<uint8_t>((sequence_id >> 24) & 0xFF));
    
    send_to_client(client_id, ack_packet);
}
```

**Format ACK** : `[0x42][0xB5][0x60][SeqID:4B]` = 7 bytes total

### 5. Implémentation handle_ack() ✅

**Localisation** : `server/src/network/UDPServer.cpp:405-424`

```cpp
void UDPServer::handle_ack(int client_id, uint32_t sequence_id) {
    std::lock_guard<std::mutex> lock(reliability_mutex_);
    
    auto it = client_reliability_.find(client_id);
    if (it == client_reliability_.end()) {
        return;
    }
    
    auto& state = it->second;
    
    // Chercher et supprimer le paquet ACKé
    for (auto pkt_it = state.pending_acks.begin(); pkt_it != state.pending_acks.end(); ++pkt_it) {
        if (pkt_it->sequence_id == sequence_id) {
            std::cout << "[Reliable] ACK received seq=" << sequence_id 
                      << " from client " << client_id
                      << " (retry_count=" << pkt_it->retry_count << ")" << std::endl;
            state.pending_acks.erase(pkt_it);
            return;
        }
    }
}
```

**Fonctionnalités** :
- ✅ Thread-safe (mutex)
- ✅ Recherche du paquet par sequence_id
- ✅ Suppression de pending_acks → Plus de retry
- ✅ Logging avec nombre de retries

### 6. Implémentation retry_unacked_packets() ✅

**Localisation** : `server/src/network/UDPServer.cpp:426-450`

```cpp
void UDPServer::retry_unacked_packets() {
    std::lock_guard<std::mutex> lock(reliability_mutex_);
    auto now = std::chrono::steady_clock::now();
    
    for (auto& [client_id, state] : client_reliability_) {
        for (auto it = state.pending_acks.begin(); it != state.pending_acks.end();) {
            if (it->should_retry(now)) {
                if (it->max_retries_reached()) {
                    std::cout << "[Warning] Packet seq=" << it->sequence_id 
                              << " to client " << client_id 
                              << " max retries reached, dropping" << std::endl;
                    it = state.pending_acks.erase(it);
                } else {
                    std::cout << "[Reliable] Retrying packet seq=" << it->sequence_id 
                              << " to client " << client_id 
                              << " (attempt " << (it->retry_count + 1) << ")" << std::endl;
                    send_to_client(client_id, it->data);
                    it->mark_resent(now);
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }
}
```

**Fonctionnalités** :
- ✅ Vérification du timeout (200ms) via `should_retry()`
- ✅ Renvoie automatique si retry_count < 3
- ✅ Drop si max retries atteint (3 tentatives)
- ✅ Mise à jour du timestamp avec `mark_resent()`
- ✅ Logging détaillé de chaque retry

### 7. Thread de retry ✅

**Localisation** : 
- Démarrage : `server/src/network/UDPServer.cpp:73` (constructeur)
- Arrêt : `server/src/network/UDPServer.cpp:78` (destructeur)
- Boucle : `server/src/network/UDPServer.cpp:452-459`

```cpp
// Constructeur
retry_thread_ = std::thread(&UDPServer::retry_thread_loop, this);

// Destructeur
if (retry_thread_.joinable()) {
    retry_thread_.join();
}

// Boucle
void UDPServer::retry_thread_loop() {
    std::cout << "[Reliable] Retry thread started" << std::endl;
    
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        retry_unacked_packets();
    }
    
    std::cout << "[Reliable] Retry thread stopped" << std::endl;
}
```

**Caractéristiques** :
- ✅ Thread séparé dédié aux retries
- ✅ Polling toutes les 50ms
- ✅ Arrêt propre avec `running_` flag
- ✅ Join dans le destructeur

### 8. Nettoyage à la déconnexion ✅

**Localisation** : `server/src/network/UDPServer.cpp:310 et 461-471`

```cpp
// Dans disconnect_client()
cleanup_client_reliability(client_id);

// Implémentation
void UDPServer::cleanup_client_reliability(int client_id) {
    std::lock_guard<std::mutex> lock(reliability_mutex_);
    
    auto it = client_reliability_.find(client_id);
    if (it != client_reliability_.end()) {
        std::cout << "[Reliable] Cleaning up reliability state for client " << client_id << std::endl;
        it->second.reset();
        client_reliability_.erase(it);
    }
}
```

**Fonctionnalités** :
- ✅ Suppression de tous les pending_acks
- ✅ Nettoyage du reorder_buffer
- ✅ Nettoyage du duplicate_cache
- ✅ Reset des sequence counters

## 📊 Statistiques et Monitoring

### Métriques à Tracker

```cpp
struct ReliabilityStats {
    uint64_t packets_sent = 0;
    uint64_t packets_acked = 0;
    uint64_t packets_retried = 0;
    uint64_t packets_lost = 0;          // Max retries atteints
    uint64_t packets_duplicated = 0;    // Doublons détectés
    uint64_t packets_reordered = 0;     // Paquets bufferisés
    
    float packet_loss_rate() const {
        return packets_sent > 0 ? (float)packets_lost / packets_sent : 0.0f;
    }
    
    float duplication_rate() const {
        return packets_sent > 0 ? (float)packets_duplicated / packets_sent : 0.0f;
    }
};
```

## 🧪 Tests Recommandés

### 1. Test Perte de Paquets (Packet Loss)

```bash
# Simuler 10% de perte avec netem (Linux)
sudo tc qdisc add dev eth0 root netem loss 10%

# Vérifier que tous les messages critiques arrivent
./bin/r-type_server &
./bin/r-type_client
# → LoginAck, StartGame doivent être reçus malgré la perte
```

### 2. Test Reordering

```bash
# Simuler reordering avec netem
sudo tc qdisc add dev eth0 root netem delay 50ms 20ms

# Envoyer 10 paquets rapides
# Vérifier qu'ils sont traités dans l'ordre
```

### 3. Test Duplication

```bash
# Simuler duplication avec netem
sudo tc qdisc add dev eth0 root netem duplicate 20%

# Vérifier que les paquets ne sont traités qu'une fois
```

## ⚡ Optimisations

### 1. **RTT Adaptatif**

Au lieu d'un timeout fixe de 200ms, calculer le RTT moyen :

```cpp
float smooth_rtt = 0.9 * smooth_rtt + 0.1 * measured_rtt;
int adaptive_timeout = smooth_rtt * 1.5; // 1.5x RTT
```

### 2. **Selective ACK (SACK)**

Au lieu d'ACKer chaque paquet, envoyer des plages :

```
ACK: seq=[5-10, 15-20] → "J'ai reçu 5-10 et 15-20, mais pas 11-14"
```

### 3. **Congestion Control**

Réduire le taux d'envoi si trop de retries :

```cpp
if (pending_acks.size() > THRESHOLD) {
    send_rate /= 2; // Réduire débit
}
```

## 📝 Résumé

✅ **Perte de Paquets** : ACK + Retransmission (3 tentatives, 200ms timeout)  
✅ **Reordering** : Fenêtre glissante 64 paquets + buffer 500ms  
✅ **Duplication** : Cache 256 entrées + TTL 5 secondes  
✅ **Stratégie Hybride** : Fiabilité uniquement pour messages critiques  

Cette implémentation offre une base solide pour un jeu multijoueur sur UDP ! 🎮
