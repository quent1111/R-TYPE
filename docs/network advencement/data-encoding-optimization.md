# Optimisation de l'Encodage des Données Réseau

## 📋 Vue d'Ensemble

Cette documentation décrit l'implémentation de la **quantization** et du **bit-packing** pour optimiser la bande passante du serveur R-TYPE. L'objectif est de réduire de **60-75%** la taille des paquets réseau tout en maintenant une précision suffisante pour le gameplay.

### Résultats

| Type de Données | Avant | Après | Économie |
|----------------|-------|-------|----------|
| Position (x, y) | 8 bytes | 4 bytes | **50%** |
| Vélocité (vx, vy) | 8 bytes | 2 bytes | **75%** |
| Santé (current, max) | 8 bytes | 2 bytes | **75%** |
| Transform complet | 16 bytes | 6 bytes | **62.5%** |
| 8 flags booléens | 8 bytes | 1 byte | **87.5%** |

---

## 🎯 Problème Initial

### Analyse de la Bande Passante (Avant)

Le serveur R-TYPE envoie les positions des entités à **60 ticks/seconde** avec un encodage naïf :

```cpp
// Ancien code (EntityBroadcaster.cpp)
broadcast_serializer_ << pos.x;      // 4 bytes (float)
broadcast_serializer_ << pos.y;      // 4 bytes (float)
broadcast_serializer_ << vx;         // 4 bytes (float)
broadcast_serializer_ << vy;         // 4 bytes (float)
broadcast_serializer_ << health;     // 4 bytes (int32)
broadcast_serializer_ << max_health; // 4 bytes (int32)
// TOTAL: 24 bytes par entité
```

**Calcul de bande passante :**
- 10 entités × 24 bytes × 60 Hz = **14.4 KB/s par client**
- 8 joueurs × 14.4 KB/s = **115 KB/s total** (≈ 920 Kbps)

### Problèmes Identifiés

1. **Précision excessive** : Les floats 32-bit offrent une précision de `±3.4×10³⁸`, inutile pour un jeu en 1920×1080
2. **Pas de compression** : Chaque valeur est envoyée brute
3. **Gaspillage pour les flags** : 1 bool = 1 byte (alors qu'il faut 1 bit)

---

## 🔧 Solution Implémentée : QuantizedSerializer

### Architecture

```
BinarySerializer (classe de base)
    ↓ hérite
QuantizedSerializer (nouvelle classe)
    ↓ utilisé par
EntityBroadcaster (diffusion des entités)
```

### Fichiers Modifiés

1. **`/src/Common/QuantizedSerializer.hpp`** (nouveau)
   - Classe principale avec méthodes de quantization
   - Helper struct `EntityFlags` pour bit-packing

2. **`/server/include/network/EntityBroadcaster.hpp`**
   - Changement du type `BinarySerializer` → `QuantizedSerializer`

3. **`/server/src/network/EntityBroadcaster.cpp`**
   - Réécriture avec appels aux méthodes quantizées

---

## 📐 Techniques de Quantization

### 1. Quantization des Positions (float → uint16)

**Principe :** Multiplier par 10 pour conserver 1 décimale de précision

```cpp
QuantizedSerializer& write_position(float x, float y) {
    if (x < 0.0f) x = 0.0f;
    if (x > 6553.5f) x = 6553.5f;
    uint16_t quantized_x = static_cast<uint16_t>(x * 10.0f);
    *this << quantized_x;

    uint16_t quantized_y = static_cast<uint16_t>(y * 10.0f);
    *this << quantized_y;

    return *this;
}

void read_position(float& x, float& y) {
    uint16_t qx, qy;
    *this >> qx >> qy;
    x = qx / 10.0f;
    y = qy / 10.0f;
}
```

**Caractéristiques :**
- **Range** : 0 à 6553.5 pixels (suffisant pour écrans 4K)
- **Précision** : 0.1 pixel (imperceptible à l'œil humain)
- **Taille** : 2 bytes au lieu de 4 bytes
- **Économie** : 50%

**Exemple :**
```
Position : (1234.56, 789.12)
Encodage : 12346, 7891 (uint16)
Décodage : (1234.6, 789.1)
Erreur   : ±0.1 pixel
```

---

### 2. Quantization des Vélocités (float → int8)

**Principe :** Diviser par 10 pour encoder des vitesses de ±127 unités/tick

```cpp
QuantizedSerializer& write_velocity(float vx, float vy) {
    // Clamp to range [-1270, 1270]
    auto quantize = [](float v) -> int8_t {
        if (v < -1270.0f) v = -1270.0f;
        if (v > 1270.0f) v = 1270.0f;
        return static_cast<int8_t>(v / 10.0f);
    };

    *this << quantize(vx);  // 1 byte
    *this << quantize(vy);  // 1 byte
    return *this;
}

void read_velocity(float& vx, float& vy) {
    int8_t qvx, qvy;
    *this >> qvx >> qvy;
    vx = qvx * 10.0f;
    vy = qvy * 10.0f;
}
```

**Caractéristiques :**
- **Range** : -1270 à +1270 pixels/seconde (à 60 FPS : ±21 px/frame)
- **Précision** : 10 pixels/sec (suffisant pour mouvement fluide)
- **Taille** : 1 byte au lieu de 4 bytes
- **Économie** : 75%

**Exemple :**
```
Vélocité : (450.0, -320.0) px/s
Encodage : 45, -32 (int8)
Décodage : (450.0, -320.0) px/s
Erreur   : ±10 px/s
```

---

### 3. Quantization de la Santé (int32 → uint8)

**Principe :** Encoder directement en pourcentage (0-255)

```cpp
QuantizedSerializer& write_quantized_health(int current, int maximum) {
    // Clamp to [0, 255]
    if (current < 0) current = 0;
    if (current > 255) current = 255;
    if (maximum < 0) maximum = 0;
    if (maximum > 255) maximum = 255;
    
    *this << static_cast<uint8_t>(current);   // 1 byte
    *this << static_cast<uint8_t>(maximum);   // 1 byte
    return *this;
}
```

**Caractéristiques :**
- **Range** : 0 à 255 points de vie
- **Précision** : 1 HP (exacte)
- **Taille** : 2 bytes au lieu de 8 bytes
- **Économie** : 75%

---

### 4. Quantization des Angles (float → uint8)

**Principe :** Encoder 360° en 256 valeurs

```cpp
QuantizedSerializer& write_quantized_angle(float angle_degrees) {
    // Normalize to [0, 360)
    while (angle_degrees < 0.0f) angle_degrees += 360.0f;
    while (angle_degrees >= 360.0f) angle_degrees -= 360.0f;
    
    // Quantize: 360° / 256 = 1.40625° per step
    uint8_t quantized = static_cast<uint8_t>((angle_degrees / 360.0f) * 256.0f);
    *this << quantized;  // 1 byte
    return *this;
}

float read_quantized_angle() {
    uint8_t quantized;
    *this >> quantized;
    return (quantized / 256.0f) * 360.0f;
}
```

**Caractéristiques :**
- **Range** : 0° à 360°
- **Précision** : 1.40625° (256 valeurs discrètes)
- **Taille** : 1 byte au lieu de 4 bytes
- **Économie** : 75%

---

### 5. Bit-Packing pour les Flags Booléens

**Principe :** Stocker 8 bools dans 1 byte

```cpp
struct EntityFlags {
    bool is_shooting      : 1;  // Bit 0
    bool has_shield       : 1;  // Bit 1
    bool has_powerup      : 1;  // Bit 2
    bool is_invulnerable  : 1;  // Bit 3
    bool is_stunned       : 1;  // Bit 4
    bool is_critical_hp   : 1;  // Bit 5
    bool reserved1        : 1;  // Bit 6
    bool reserved2        : 1;  // Bit 7
    
    uint8_t pack() const {
        return (is_shooting      ? 0x01 : 0) |
               (has_shield       ? 0x02 : 0) |
               (has_powerup      ? 0x04 : 0) |
               (is_invulnerable  ? 0x08 : 0) |
               (is_stunned       ? 0x10 : 0) |
               (is_critical_hp   ? 0x20 : 0) |
               (reserved1        ? 0x40 : 0) |
               (reserved2        ? 0x80 : 0);
    }
    
    void unpack(uint8_t packed) {
        is_shooting      = (packed & 0x01) != 0;
        has_shield       = (packed & 0x02) != 0;
        has_powerup      = (packed & 0x04) != 0;
        is_invulnerable  = (packed & 0x08) != 0;
        is_stunned       = (packed & 0x10) != 0;
        is_critical_hp   = (packed & 0x20) != 0;
        reserved1        = (packed & 0x40) != 0;
        reserved2        = (packed & 0x80) != 0;
    }
};

// Utilisation
QuantizedSerializer& write_packed_flags(const bool* flags, size_t count) {
    uint8_t packed = 0;
    for (size_t i = 0; i < count && i < 8; ++i) {
        if (flags[i]) {
            packed |= (1 << i);
        }
    }
    *this << packed;  // 1 byte
    return *this;
}
```

**Caractéristiques :**
- **Capacité** : 8 flags booléens
- **Taille** : 1 byte au lieu de 8 bytes
- **Économie** : 87.5%

**Exemple :**
```
Flags : [true, false, true, false, false, true, false, false]
Encodage : 0b00100101 = 0x25 (1 byte)
Décodage : [true, false, true, false, false, true, false, false]
```

---

## 🚀 Utilisation dans EntityBroadcaster

### Code Avant (Non Optimisé)

```cpp
void EntityBroadcaster::broadcast_entity_positions(...) {
    broadcast_serializer_ << pos.x;           // 4 bytes
    broadcast_serializer_ << pos.y;           // 4 bytes
    broadcast_serializer_ << vx;              // 4 bytes
    broadcast_serializer_ << vy;              // 4 bytes
    broadcast_serializer_ << current_health;  // 4 bytes
    broadcast_serializer_ << max_health;      // 4 bytes
    // TOTAL: 24 bytes par entité
}
```

### Code Après (Optimisé)

```cpp
void EntityBroadcaster::broadcast_entity_positions(...) {
    // Position + Vélocité : 6 bytes au lieu de 16
    broadcast_serializer_.write_position(pos.x, pos.y);        // 4 bytes
    broadcast_serializer_.write_velocity(vx, vy);              // 2 bytes
    
    // Santé : 2 bytes au lieu de 8
    broadcast_serializer_.write_quantized_health(
        current_health, max_health);                           // 2 bytes
    
    // TOTAL: 8 bytes par entité (66% d'économie)
}
```

### Méthode Helper : write_entity_transform()

Pour simplifier l'encodage, une méthode helper combine position + vélocité :

```cpp
broadcast_serializer_.write_entity_transform(pos.x, pos.y, vx, vy);
// Équivalent à :
// write_position(pos.x, pos.y);
// write_velocity(vx, vy);
```

---

## 📊 Impact sur la Bande Passante

### Calcul Détaillé

**Paquet réseau typique :**

```
AVANT (24 bytes par entité) :
- Magic number     : 2 bytes
- OpCode           : 1 byte
- Entity count     : 1 byte
- Entity data      : 10 entités × 24 bytes = 240 bytes
TOTAL              : 244 bytes/paquet

APRÈS (8 bytes par entité) :
- Magic number     : 2 bytes
- OpCode           : 1 byte
- Entity count     : 1 byte
- Entity data      : 10 entités × 8 bytes = 80 bytes
TOTAL              : 84 bytes/paquet

ÉCONOMIE           : 160 bytes/paquet (65.6%)
```

### Bande Passante Réseau

| Scénario | Avant | Après | Économie |
|----------|-------|-------|----------|
| 1 client, 10 entités @ 60Hz | 14.4 KB/s | 4.9 KB/s | **66%** |
| 8 clients, 10 entités @ 60Hz | 115 KB/s | 39 KB/s | **66%** |
| 16 clients, 20 entités @ 60Hz | 460 KB/s | 157 KB/s | **66%** |

**Bénéfices :**
- ✅ Réduit la latence réseau
- ✅ Supporte plus de joueurs simultanés
- ✅ Fonctionne mieux sur connexions lentes (mobile, wifi)
- ✅ Réduit les coûts de bande passante serveur

---

## 🎮 Précision vs Performance

### Compromis de Précision

| Type | Précision | Visible en jeu ? | Justification |
|------|-----------|------------------|---------------|
| Position (±0.1 px) | Excellente | ❌ Non | Pixel imperceptible à 1920x1080 |
| Vélocité (±10 px/s) | Bonne | ❌ Non | Mouvement reste fluide @ 60 FPS |
| Angle (±1.4°) | Bonne | ⚠️ Légèrement | Acceptable pour rotation visuelle |
| Santé (±1 HP) | Exacte | ❌ Non | Pas de perte de précision |

### Tests Recommandés

```cpp
// Test de round-trip precision
void test_position_quantization() {
    float original_x = 1234.567f;
    float original_y = 890.123f;
    
    // Encode
    QuantizedSerializer serializer;
    serializer.write_position(original_x, original_y);
    
    // Decode
    float decoded_x, decoded_y;
    serializer.read_position(decoded_x, decoded_y);
    
    // Vérifier erreur < 0.1 pixel
    assert(std::abs(decoded_x - original_x) < 0.1f);
    assert(std::abs(decoded_y - original_y) < 0.1f);
}
```

---

## 🔄 Décodage Côté Client

### Implémentation Requise

Le client doit utiliser les **mêmes méthodes de déquantization** :

```cpp
// Dans le client (networkClient.cpp ou similaire)
void decode_entity_positions(const std::vector<uint8_t>& data) {
    RType::QuantizedSerializer deserializer(data);
    
    // Skip header
    uint16_t magic;
    uint8_t opcode, count;
    deserializer >> magic >> opcode >> count;
    
    for (uint8_t i = 0; i < count; ++i) {
        uint32_t entity_id;
        uint8_t entity_type;
        deserializer >> entity_id >> entity_type;
        
        // 🔓 DÉQUANTIZATION
        float x, y, vx, vy;
        deserializer.read_position(x, y);
        deserializer.read_velocity(vx, vy);
        
        int current_health, max_health;
        deserializer.read_quantized_health(current_health, max_health);
        
        // Update entity in local registry
        update_entity(entity_id, x, y, vx, vy, current_health, max_health);
    }
}
```

### ⚠️ Important : Synchronisation

**Le client et le serveur doivent utiliser la même version de `QuantizedSerializer`** :
- Même ordre de lecture/écriture
- Mêmes facteurs de quantization (×10 pour position, ÷10 pour vélocité)
- Mêmes ranges de clamping

**Recommandation :** Partager `QuantizedSerializer.hpp` entre client et serveur via un module commun.

---

## 🧪 Tests et Validation

### Tests Unitaires Requis

1. **Test de précision** (fichier à créer : `tests/network/test_quantization.cpp`)

```cpp
TEST(QuantizedSerializer, PositionPrecision) {
    RType::QuantizedSerializer ser;
    
    // Test plusieurs positions
    std::vector<std::pair<float, float>> positions = {
        {0.0f, 0.0f},
        {1920.0f, 1080.0f},
        {1234.56f, 789.12f},
        {6553.5f, 6553.5f}  // Max range
    };
    
    for (const auto& [x, y] : positions) {
        ser.clear();
        ser.write_position(x, y);
        
        float decoded_x, decoded_y;
        ser.read_position(decoded_x, decoded_y);
        
        EXPECT_NEAR(decoded_x, x, 0.1f);
        EXPECT_NEAR(decoded_y, y, 0.1f);
    }
}

TEST(QuantizedSerializer, VelocityRange) {
    RType::QuantizedSerializer ser;
    
    // Test range limits
    ser.write_velocity(-1270.0f, 1270.0f);
    
    float vx, vy;
    ser.read_velocity(vx, vy);
    
    EXPECT_FLOAT_EQ(vx, -1270.0f);
    EXPECT_FLOAT_EQ(vy, 1270.0f);
}

TEST(EntityFlags, BitPacking) {
    RType::EntityFlags flags;
    flags.is_shooting = true;
    flags.has_shield = false;
    flags.has_powerup = true;
    
    uint8_t packed = flags.pack();
    EXPECT_EQ(packed, 0b00000101);
    
    RType::EntityFlags unpacked;
    unpacked.unpack(packed);
    
    EXPECT_TRUE(unpacked.is_shooting);
    EXPECT_FALSE(unpacked.has_shield);
    EXPECT_TRUE(unpacked.has_powerup);
}
```

2. **Test d'intégration** (client-serveur)

```bash
# Lancer serveur + client en local
# Capturer paquets réseau avec Wireshark
# Vérifier taille des paquets EntityPosition

# Avant : ~244 bytes/paquet
# Après  : ~84 bytes/paquet
```

---

## 📈 Prochaines Étapes (Phase 2 & 3)

### Phase 2 : Fiabilité Réseau

- [ ] **Numéros de séquence** : Détecter paquets perdus/dupliqués
- [ ] **Système ACK** : Confirmation de réception
- [ ] **Reorder buffer** : Gérer paquets hors ordre
- [ ] **Interpolation** : Lisser les positions manquantes

### Phase 3 : Compression

- [ ] **Delta encoding** : Envoyer uniquement les changements
- [ ] **LZ4 compression** : Compresser paquets volumineux
- [ ] **Snapshot system** : État complet + deltas

### Bande Passante Cible Finale

| Phase | Bytes/Entité | Économie Totale |
|-------|-------------|-----------------|
| Phase 1 (actuelle) | 8 bytes | 66% |
| Phase 2 (séquence) | 10 bytes | 58% |
| Phase 3 (delta) | 3-5 bytes | 80-90% |

---

## 🔗 Références

### Fichiers Modifiés

```
src/Common/QuantizedSerializer.hpp          (nouveau, 362 lignes)
server/include/network/EntityBroadcaster.hpp (modifié)
server/src/network/EntityBroadcaster.cpp     (modifié)
```

### Ressources Externes

- [Gaffer on Games - State Synchronization](https://gafferongames.com/post/state_synchronization/)
- [Valve - Source Multiplayer Networking](https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking)
- [Overwatch - Netcode Analysis](https://www.youtube.com/watch?v=vTH2ZPgYujQ)

### Documentation Interne

- `/docs/architecture/network.md` - Architecture réseau globale
- `/docs/developer-guide/testing.md` - Guide de tests
- `/README.md` - Instructions de build

---

## ✅ Checklist de Déploiement

Avant de merger cette feature :

- [x] ✅ `QuantizedSerializer.hpp` créé et compilé
- [x] ✅ `EntityBroadcaster` mis à jour
- [x] ✅ Code serveur compile sans erreurs
- [ ] ⏳ Client mis à jour avec déquantization
- [ ] ⏳ Tests unitaires écrits et passés
- [ ] ⏳ Tests d'intégration client-serveur
- [ ] ⏳ Mesures de bande passante validées
- [ ] ⏳ Documentation API générée (Doxygen)
- [ ] ⏳ Merge request créée et reviewée

---

## 📞 Contact

Pour questions ou problèmes :
- **Branch** : `network-track`
- **Pull Request** : #TBD
- **Documentation** : `/docs/network/`

**Auteur** : Équipe R-TYPE - Optimisation Réseau  
**Date** : Janvier 2026  
**Version** : 1.0.0
