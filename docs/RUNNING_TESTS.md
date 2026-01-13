# 🧪 Guide d'Exécution des Tests R-TYPE

Ce document explique comment compiler et exécuter tous les tests du projet R-TYPE.

---

## 📋 Table des Matières

1. [Prérequis](#prérequis)
2. [Compilation des Tests](#compilation-des-tests)
3. [Exécution des Tests](#exécution-des-tests)
4. [Tests Disponibles](#tests-disponibles)
5. [Interprétation des Résultats](#interprétation-des-résultats)
6. [Débogage](#débogage)

---

## ✅ Prérequis

- Projet R-TYPE déjà compilé avec succès
- CMake configuré
- Google Test (gtest) installé via Conan
- Compilateur C++20 compatible

---

## 🔨 Compilation des Tests

### Option 1 : Utiliser le script r-type.sh (Recommandé)

```bash
./r-type.sh build
```

Cela compile automatiquement :
- Le projet principal
- Tous les exécutables de tests
- Les dépendances

### Option 2 : Compilation manuelle avec CMake

```bash
cd build/build/Release
cmake --build . --target test_network
cmake --build . --target test_game
```

---

## 🚀 Exécution des Tests

### 1️⃣ Tests Réseau (Network Tests)

**Contenu :**
- ✅ test_input_buffer.cpp (25+ tests)
- ✅ test_packet_reliability.cpp (45+ tests)
- ✅ Tests existants (protocol, serialization, compression)

**Lancer :**

```bash
# Depuis la racine du projet
./build/build/Release/tests/test_network

# Ou avec CTest
cd build/build/Release
ctest -R NetworkTests --verbose
```

**Ce qui est testé :**
- 🔄 Input delaying (buffer 50ms)
- 📦 Fiabilité UDP (ACK, retry, reordering)
- 🔢 Sérialisation/désérialisation
- 🗜️ Compression LZ4
- 🌐 Protocole réseau

---

### 2️⃣ Tests de Jeu (Game Tests)

**Contenu :**
- ✅ test_client_prediction.cpp (50+ tests)
- ✅ test_position_history.cpp (55+ tests)
- ✅ Tests existants (collision, movement, weapons)

**Lancer :**

```bash
# Depuis la racine du projet
./build/build/Release/tests/test_game

# Ou avec CTest
cd build/build/Release
ctest -R GameTests --verbose
```

**Ce qui est testé :**
- 🎯 Prédiction client (smooth + snap)
- 📜 Historique de position (buffer circulaire 60 frames)
- 💥 Collision
- 🚀 Mouvement
- ⚔️ Armes et power-ups

---

### 3️⃣ Tous les Tests

**Lancer tous les tests en une fois :**

```bash
cd build/build/Release
ctest --output-on-failure

# Ou avec plus de détails
ctest --verbose
```

---

## 📊 Tests Disponibles

### Tests Réseau (test_network)

| Fichier | Tests | Description |
|---------|-------|-------------|
| `test_input_buffer.cpp` | 25+ | Buffer d'input avec délai de 50ms |
| `test_packet_reliability.cpp` | 45+ | Fiabilité UDP (ACK, retry, reordering) |
| `test_serialization.cpp` | 20+ | Sérialisation binaire |
| `test_compression.cpp` | 15+ | Compression LZ4 |
| **TOTAL** | **~120 tests** | |

### Tests de Jeu (test_game)

| Fichier | Tests | Description |
|---------|-------|-------------|
| `test_client_prediction.cpp` | 50+ | Prédiction client (smooth/snap) |
| `test_position_history.cpp` | 55+ | Buffer circulaire de positions |
| `test_collision.cpp` | 10+ | Système de collision |
| `test_movement.cpp` | 10+ | Système de mouvement |
| `test_weapon.cpp` | 15+ | Armes et tirs |
| **TOTAL** | **~150 tests** | |

---

## 📈 Interprétation des Résultats

### Sortie Typique (Succès)

```
[==========] Running 70 tests from 10 test suites.
[----------] Global test environment set-up.
[----------] 25 tests from InputBufferTest
[ RUN      ] InputBufferTest.AddSingleInput
[       OK ] InputBufferTest.AddSingleInput (0 ms)
...
[----------] 25 tests from InputBufferTest (15 ms total)

[==========] 70 tests from 10 test suites ran. (250 ms total)
[  PASSED  ] 70 tests.

[Performance] 10000 add_input() took 842µs
[Performance] 1000 get_ready_inputs() took 123µs
```

### Résultats de Performance Attendus

**Input Buffer :**
- ✅ 10000 add_input() : < 10ms
- ✅ 1000 get_ready_inputs() : < 1ms

**Packet Reliability :**
- ✅ 1000 process_received_packet() : < 10ms
- ✅ Reordering de 100 paquets : < 5ms

**Client Prediction :**
- ✅ 10000 apply_prediction() : < 1ms
- ✅ 10000 apply_correction() : < 1ms

**Position History :**
- ✅ 10000 add_position() : < 5ms
- ✅ 10000 get_delayed_position() : < 5ms

---

## 🐛 Débogage

### En cas d'échec de test

#### 1. Voir les détails de l'échec

```bash
ctest --output-on-failure
```

#### 2. Lancer un test spécifique

```bash
# Test réseau uniquement
./build/build/Release/tests/test_network --gtest_filter="InputBufferTest.*"

# Test de prédiction client uniquement
./build/build/Release/tests/test_game --gtest_filter="ClientPredictionTest.*"
```

#### 3. Mode verbeux Google Test

```bash
./build/build/Release/tests/test_network --gtest_print_time=1 --gtest_color=yes
```

#### 4. Lister tous les tests sans les exécuter

```bash
./build/build/Release/tests/test_network --gtest_list_tests
```

---

## 🔍 Tests Spécifiques

### Tester uniquement l'Input Delaying

```bash
./build/build/Release/tests/test_network --gtest_filter="InputBufferTest.*"
```

**Résultat attendu :** 25 tests passent

---

### Tester uniquement la Fiabilité UDP

```bash
./build/build/Release/tests/test_network --gtest_filter="PacketReliabilityTest.*"
```

**Résultat attendu :** 45 tests passent

---

### Tester uniquement la Prédiction Client

```bash
./build/build/Release/tests/test_game --gtest_filter="ClientPredictionTest.*"
```

**Résultat attendu :** 50 tests passent

---

### Tester uniquement l'Historique de Position

```bash
./build/build/Release/tests/test_game --gtest_filter="PositionHistoryTest.*"
```

**Résultat attendu :** 55 tests passent

---

## 🎯 Exemples de Commandes Utiles

### Exécuter seulement les tests rapides (skip tests longs)

```bash
./build/build/Release/tests/test_network --gtest_filter=-*Performance*:-*Stress*
```

### Exécuter seulement les tests de performance

```bash
./build/build/Release/tests/test_network --gtest_filter=*Performance*
```

### Répéter un test 100 fois (pour détecter race conditions)

```bash
./build/build/Release/tests/test_network --gtest_filter="InputBufferTest.ConcurrentAccess" --gtest_repeat=100
```

### Exécuter tests en parallèle avec CTest

```bash
cd build/build/Release
ctest -j4  # 4 jobs en parallèle
```

---

## 📝 Notes Importantes

### Tests Désactivés Temporairement

Certains tests ont des sections commentées car trop longues :
- `InputBufferTest.ExpiredInputsRemoved` (5100ms timeout)
- `DuplicateCacheEntry.Expiration` (5000ms)

Pour les activer, décommentez les `sleep_for` dans les fichiers de test.

### Tests Nécessitant des Includes Réels

Les tests suivants utilisent des structures mockées :
- `test_client_prediction.cpp` (mock de `PredictionState`)
- `test_position_history.cpp` (mock de `CircularBuffer`)

Pour tester avec les vraies structures :
1. Remplacer les mocks par les includes réels
2. Adapter les appels de fonction selon l'API réelle

---

## ✅ Checklist de Validation

Avant de merger une branche, vérifier que :

- [ ] `./r-type.sh build` compile sans erreur
- [ ] `ctest` passe tous les tests (NetworkTests + GameTests)
- [ ] Aucun leak mémoire détecté (si valgrind activé)
- [ ] Tests de performance dans les limites attendues
- [ ] Aucun test désactivé sans raison valable

---

## 📞 Support

En cas de problème :
1. Vérifier que le projet compile : `./r-type.sh build`
2. Vérifier les logs de CMake dans `build/build/Release/`
3. Lancer les tests en mode verbeux : `ctest --verbose`
4. Consulter la documentation des systèmes testés :
   - `docs/INPUT_DELAYING_IMPLEMENTATION.md`
   - `docs/RELIABLE_UDP_IMPLEMENTATION.md`
   - `docs/ROLLBACK_REPLAY_SYSTEM.md`

---

## 🎉 Résumé Rapide

```bash
# Compiler tout
./r-type.sh build

# Lancer tous les tests
cd build/build/Release && ctest --output-on-failure

# Lancer tests réseau (120+ tests)
./build/build/Release/tests/test_network

# Lancer tests de jeu (150+ tests)
./build/build/Release/tests/test_game

# Tests spécifiques
./build/build/Release/tests/test_network --gtest_filter="InputBufferTest.*"
./build/build/Release/tests/test_network --gtest_filter="PacketReliabilityTest.*"
./build/build/Release/tests/test_game --gtest_filter="ClientPredictionTest.*"
./build/build/Release/tests/test_game --gtest_filter="PositionHistoryTest.*"
```

**Total : ~270 tests couvrant tous les systèmes de networking ! 🚀**
