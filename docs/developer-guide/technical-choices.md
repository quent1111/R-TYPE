# Justification des Choix Techniques

Ce document détaille et justifie **tous les choix techniques majeurs** du projet R-TYPE avec des comparaisons objectives entre alternatives.

---

## Table des Matières

- [Langage et Standard](#langage-et-standard)
  - C++ (imposé) + C++20
- [Gestion des Dépendances](#gestion-des-dépendances)
  - Conan 2.x
- [Bibliothèques Graphiques](#bibliothèques-graphiques)
  - SFML 2.6.1
- [Bibliothèques Réseau](#bibliothèques-réseau)
  - Asio 1.30.2 + UDP
- [Architecture Logicielle](#architecture-logicielle)
  - ECS pattern + Custom implementation + Sparse Array
- [Outils de Build](#outils-de-build)
  - CMake 3.20+ + Scripts unifiés (r-type.sh/bat)
- [Testing](#testing)
  - GTest 1.14.0 + lcov/gcov
- [Qualité de Code](#qualité-de-code)
  - clang-format + Google Style (modifié) + clang-tidy
- [Documentation](#documentation)
  - MkDocs + Material theme + GitHub Pages
- [CI/CD](#cicd)
  - GitHub Actions + workflows (ci, pr-check, coverage)
- [Plateformes Supportées](#plateformes-supportées)
  - Linux + macOS + Windows + Compilateurs (GCC 10+, Clang 12+, MSVC 2019+)

---

## Langage et Standard

### Langage de Programmation : C++

**Choix imposé** : C++ est obligatoire pour le projet R-TYPE.

**Avantages pour R-TYPE** :
- Performance proche du métal pour un jeu temps réel 60 FPS
- Contrôle précis de la mémoire et des ressources
- Écosystème mature pour le développement de jeux (SFML, SDL)
- Compatibilité multiplateforme native

### Version du Standard : C++20

**Justification** : C++20 offre un équilibre optimal entre modernité et stabilité.

**Comparatif** :

| Critère | C++17 | **C++20** ✅ | C++23 |
|---------|-------|-------------|-------|
| Support compilateurs | Excellent | Excellent | Limité |
| Features modernes | Bonnes | **Excellentes** | Expérimentales |
| Stabilité | Très stable | Stable | En évolution |
| Concepts | ❌ | ✅ | ✅ |
| Ranges | Partiel | ✅ | Amélioré |

**Features C++20 utilisées dans l'ECS** :
- **Fold expressions** : `(++std::get<Is>(_current), ...)` dans zipper_iterator pour incrémenter tous les itérateurs
- **Fold expressions avec &&** : `((*std::get<Is>(_current)).has_value() && ...)` pour vérifier si tous les composants sont présents
- **`constexpr` amélioré** : Fonctions constexpr pour entity (`operator==`, `operator<`, `id()`) permettant des calculs compile-time
- **`std::index_sequence_for`** : Génération d'index sequences pour itérer sur les packs de templates (`static constexpr std::index_sequence_for<Containers...> _seq{}`)
- **Designated initializers** : Initialisation claire des composants (`position{.x = 100.0f, .y = 200.0f}`)
- **Template parameter packs améliorés** : Utilisation extensive dans zipper_iterator et indexed_zipper

**Exemples concrets dans le code** :
```cpp
// Fold expression (C++20) - zipper_iterator.hpp:121
return ((*std::get<Is>(_current)).has_value() && ...);

// constexpr amélioré (C++20) - entity.hpp:14-20
constexpr explicit operator std::size_t() const noexcept { return _id; }
constexpr bool operator==(entity const& other) const noexcept { return _id == other._id; }

// index_sequence_for (C++17+) - zipper_iterator.hpp:135
static constexpr std::index_sequence_for<Containers...> _seq{};
```

**Pourquoi pas C++23** : Support limité des compilateurs en production (GCC 13+, Clang 16+), nos features C++20 suffisent

---

## Gestion des Dépendances

### Package Manager : Conan 2.x

**Justification** : Conan est le gestionnaire de paquets le plus adapté pour un projet C++ multiplateforme.

**Comparatif** :

| Critère | Vcpkg | **Conan 2.x** ✅ |
|---------|-------|------------------|
| Intégration CMake | Native | Excellente (toolchain) |
| Gestion binaires | ✅ | ✅ Avec cache |
| Configurations multiples | Limitée | **Flexible (profiles)** |
| Cross-compilation | Complexe | **Simple** |
| Communauté C++ | Microsoft-centric | **Large communauté** |
| Recettes custom | Difficile | **Facile** |

**Avantages pour R-TYPE** :
- **Conan Center** héberge SFML, Asio, GTest de manière fiable
- **Profiles** permettent de gérer Debug/Release facilement
- **Cache des binaires** accélère les builds en CI/CD
- **Configuration déclarative** avec `conanfile.py`
- Support natif des **system packages** (X11 sur Linux)

**Résultat** : Installation automatisée des dépendances avec `./r-type.sh` en une seule commande

---

## Bibliothèques Graphiques

### Bibliothèque Graphique : SFML 2.6.1

**Justification** : SFML offre le meilleur équilibre simplicité/performance pour un shoot'em up 2D.

**Comparatif** :

| Critère | SDL2 | **SFML 2.6.1** ✅ | Raylib | OpenGL direct |
|---------|------|-------------------|--------|---------------|
| Courbe d'apprentissage | Moyenne | **Facile** | Facile | Difficile |
| API C++ native | ❌ (C) | **✅** | ❌ (C) | ❌ (C) |
| Gestion sprites | Manuelle | **Intégrée** | Basique | Manuelle |
| Audio intégré | ❌ (extension) | **✅** | ✅ | ❌ |
| Animations | Manuelle | **sf::IntRect** | Manuelle | Manuelle |
| Performance 2D | Excellente | **Excellente** | Bonne | Maximale |
| Multiplateforme | ✅ | **✅** | ✅ | ✅ |
| Documentation | Bonne | **Excellente** | Bonne | Complexe |

**Avantages pour R-TYPE** :
- **API orientée objet** naturelle en C++ (`sf::Sprite`, `sf::Texture`, `sf::RenderWindow`)
- **Gestion des sprites simplifiée** avec `sf::IntRect` pour les spritesheets
- **Audio intégré** pour musique et effets sonores (futur)
- **Système de fenêtre** avec gestion des événements clés en main
- **Performance optimale** pour le rendu 2D avec OpenGL en backend
- **Communauté active** et documentation riche

**Pourquoi pas SDL2** : API C nécessitant plus de wrapping, gestion audio externe
**Pourquoi pas Raylib** : Moins mature, communauté plus petite
**Pourquoi pas OpenGL direct** : Complexité excessive pour un jeu 2D simple

---

## Bibliothèques Réseau

### Bibliothèque Réseau : Asio 1.30.2

**Justification** : Asio standalone offre une API réseau asynchrone moderne sans la lourdeur de Boost.

**Comparatif** :

| Critère | Boost.Asio | **Asio 1.30.2** ✅ | ENet | Raw Sockets |
|---------|------------|-------------------|------|-------------|
| Dépendances | **Tout Boost** | **Header-only** | Légère | Aucune |
| API moderne | C++11 | **C++20** | C | Bas niveau |
| Async I/O | ✅ | **✅** | ✅ | Manuel |
| Multiplateforme | ✅ | **✅** | ✅ | Complexe |
| UDP + TCP | ✅ | **✅** | UDP only | ✅ |
| Taille compile | Lourde | **Légère** | Légère | Minimale |
| Documentation | Excellente | **Excellente** | Moyenne | Standard OS |

**Avantages pour R-TYPE** :
- **Version standalone** : Pas besoin d'installer tout Boost (~100+ MB)
- **API asynchrone** : `async_receive_from` pour gérer plusieurs clients sans threads
- **io_context** : Event loop performant pour le serveur de jeu
- **Support UDP natif** : `asio::ip::udp::socket` avec endpoints
- **Portabilité** : Abstraction uniforme Windows/Linux/macOS

**Pourquoi pas Boost.Asio** : Dépendance trop lourde, temps de compilation long
**Pourquoi pas ENet** : Limité à UDP, moins flexible pour mix TCP/UDP futur
**Pourquoi pas Raw Sockets** : Réinventer la roue, gestion des erreurs complexe

### Protocole Réseau : UDP

**Justification** : UDP est le protocole optimal pour un jeu d'action en temps réel.

**Comparatif** :

| Critère | TCP | **UDP** ✅ |
|---------|-----|-----------|
| Latence | 100-200ms (handshake + ACK) | **10-50ms** |
| Overhead | En-tête + retransmission | **Minimal** |
| Ordre des paquets | Garanti | Non garanti |
| Fiabilité | 100% | ~95-99% |
| Idéal pour | Chat, téléchargement | **Jeux FPS, shoot'em up** |

**Avantages pour R-TYPE (shoot'em up 60 FPS)** :
- **Latence ultra-faible** : Chaque milliseconde compte pour les tirs
- **Pas de Head-of-Line Blocking** : Un paquet perdu ne bloque pas les suivants
- **Tolérance aux pertes** : Perdre une frame de position n'est pas critique (la suivante arrive 16ms après)
- **Simplicité** : Pas de gestion de connexion persistante
- **Scalabilité** : Un socket peut gérer tous les clients

**Mix TCP + UDP envisagé** :
- **TCP** : Connexion initiale, authentification, chat (futur)
- **UDP** : Positions, tirs, updates 60 FPS

---

## Architecture Logicielle

### Pattern Architectural : ECS (Entity-Component-System)

**Justification** : L'ECS maximise la performance grâce à une architecture orientée données.

**Comparatif** :

| Critère | OOP Classique | Observer Pattern | **ECS** ✅ |
|---------|---------------|------------------|-----------|
| Cache locality | Faible | Moyenne | **Excellente** |
| Couplage | Fort | Moyen | **Minimal** |
| Flexibilité | Rigide (héritage) | Moyenne | **Maximale** |
| Performance | Bonne | Bonne | **Optimale** |
| Lisibilité | Intuitive | Moyenne | **Clara (data/logic séparés)** |
| Réutilisation | Limitée | Bonne | **Excellente** |

**Avantages pour R-TYPE** :
- **Performance** : Itérer sur 1000 entités = accès séquentiel mémoire (vs pointeurs éparpillés)
- **Composition** : Créer un ennemi = `position` + `velocity` + `sprite` + `health` (pas d'héritage complexe)
- **Systems découplés** : `movement_system` ne sait pas ce qu'est un "joueur" ou "ennemi"
- **Hot-reload friendly** : Modifier un component ne casse pas tout

**Pourquoi pas OOP** : `Player : public Entity`, `Enemy : public Entity` crée des hiérarchies rigides
**Pourquoi pas Observer** : Événements omniprésents = overhead, difficile à debug

### Implémentation ECS : Custom ECS from Scratch

**Justification** : Implémenter notre ECS garantit la compréhension totale et le contrôle.

**Comparatif** :

| Critère | EnTT | flecs | **Custom ECS** ✅ |
|---------|------|-------|------------------|
| Performances | Excellentes | Excellentes | **Bonnes (optimisables)** |
| Complexité | Élevée | Élevée | **Simple** |
| Contrôle | Limité | Limité | **Total** |
| Dépendances | 1 bibliothèque | 1 bibliothèque | **0** |
| Apprentissage | Courbe raide | Courbe raide | **Pédagogique** |
| Template magic | +++++ | ++++ | **+** |
| Taille compile | Lourde | Lourde | **Légère** |

**Avantages pour R-TYPE (projet académique)** :
- **Compréhension** : Chaque ligne de `registry.hpp` est maîtrisée
- **Simplicité** : ~500 lignes de code au lieu de 50k (EnTT)
- **Debugging** : Pas de métaprogrammation obscure
- **Adaptabilité** : Modifier `sparse_array` si besoin spécifique

**Pourquoi pas EnTT** : Production-ready mais opaque (templates extrêmes)
**Pourquoi pas flecs** : Excellent mais trop de features pour notre scope

### Structure de Données : Sparse Array avec `std::optional<T>`

**Justification** : Le `sparse_array<T>` équilibre performance et flexibilité.

**Comparatif** :

| Critère | Dense Array | Hash Map | **Sparse Array** ✅ |
|---------|-------------|----------|---------------------|
| Accès O() | **O(1)** | O(1) | **O(1)** |
| Itération | **Rapide** | Lente | **Rapide** |
| Mémoire | **Optimale** | Élevée | **Raisonnable** |
| Trous (entités mortes) | Problème | OK | **Géré (std::optional)** |
| Cache-friendly | **+++** | + | **++** |
| Complexité code | Reindexing requis | Simple | **Simple** |

**Avantages pour R-TYPE** :
- **Accès direct** : `components[entity_id]` = O(1) garanti
- **Itération cache-friendly** : Array contigu en mémoire
- **Entités réutilisées** : `std::optional` marque les slots vides sans réorganiser
- **Simplicité** : Pas besoin de hash function ou collision handling

**Implémentation clé** :
```cpp
template<typename T>
class sparse_array {
    std::vector<std::optional<T>> _data;
    // operator[] redimensionne automatiquement
};
```

**Pourquoi pas Dense Array** : Réindexer toutes les entités lors d'une suppression = complexe
**Pourquoi pas Hash Map** : Itération lente, cache misses fréquents

---

## Outils de Build

### Système de Build : CMake 3.20+

**Justification** : CMake est le standard de facto pour les projets C++ multiplateformes.

**Comparatif** :

| Critère | Makefile | Premake | Meson | **CMake 3.20+** ✅ |
|---------|----------|---------|-------|-------------------|
| Multiplateforme | Limité | Bon | Excellent | **Excellent** |
| IDE support | Faible | Moyen | Bon | **Excellent (CLion, VSCode)** |
| Conan integration | Manuelle | Limitée | Bonne | **Excellente (toolchain)** |
| Communauté | Moyenne | Petite | Croissante | **Énorme** |
| Syntaxe | Shell | Lua | Python-like | **CMake script** |
| Courbe apprentissage | Moyenne | Facile | Moyenne | **Raide (mais documenté)** |

**Avantages pour R-TYPE** :
- **Conan toolchain** : `cmake_layout()` + `CMakeToolchain` automatiques
- **Target-based** : `target_link_libraries(r-type_client PRIVATE SFML::Graphics)`
- **Multi-config** : Debug/Release dans le même build
- **CTest intégré** : `./r-type.sh test` utilise CTest
- **find_package** : Détection automatique des dépendances

**Pourquoi pas Makefile** : Gestion manuelle des dépendances, pas de multi-plateforme
**Pourquoi pas Premake** : Communauté trop petite, moins de support IDE
**Pourquoi pas Meson** : Jeune, moins d'intégration Conan

### Scripts de Build Unifiés : r-type.sh / r-type.bat

**Justification** : Des scripts unifiés simplifient drastiquement le workflow développeur.

**Comparatif** :

| Critère | Commandes manuelles | **Scripts unifiés** ✅ |
|---------|---------------------|------------------------|
| Courbe apprentissage | Élevée | **Minimale** |
| Gestion dépendances | Manuelle | **Automatique (Conan)** |
| Multi-config | Complexe | **1 commande** |
| Erreurs utilisateur | Fréquentes | **Rares** |
| Documentation | Longue | **`./r-type.sh --help`** |

**Commandes simplifiées** :
```bash
./r-type.sh build          # Au lieu de: conan install + cmake + make
./r-type.sh server         # Au lieu de: build + cd build/.../bin + ./r-type_server
./r-type.sh test           # Au lieu de: build + ctest --test-dir build/...
./r-type.sh coverage       # Au lieu de: test + gcov + lcov + genhtml
```

**Avantages pour R-TYPE (projet étudiant)** :
- **Onboarding rapide** : Nouveaux devs opérationnels en 1 commande
- **Cross-platform** : `.sh` pour Unix, `.bat` pour Windows (même syntaxe)
- **Options standardisées** : `--debug`, `--release`, `-j 8`, `--clean`
- **Gestion d'erreurs** : Vérification des prérequis (CMake, Conan, GCC/Clang)

**Pourquoi pas commandes manuelles** : 15+ commandes vs 1, erreurs fréquentes

---

## Testing

### Framework de Tests : Google Test (GTest) 1.14.0

**Justification** : GTest est le framework de test C++ le plus populaire et le mieux intégré.

**Comparatif** :

| Critère | Catch2 | Doctest | Boost.Test | **GTest 1.14.0** ✅ |
|---------|--------|---------|------------|---------------------|
| Syntaxe | BDD-style | Simple | Complexe | **Macros classiques** |
| Compilation | Lente | **Rapide** | Lente | Moyenne |
| Mocking | Nécessite ext. | Nécessite ext. | Limité | **GMock intégré** |
| Documentation | Bonne | Moyenne | Moyenne | **Excellente** |
| Adoption | Croissante | Petite | Moyenne | **Industrie standard** |
| CMake support | Bon | Bon | Moyen | **Excellent** |
| Assertions | Riches | Basiques | Riches | **Très riches** |

**Avantages pour R-TYPE** :
- **EXPECT_* macros** : `EXPECT_EQ`, `EXPECT_FLOAT_EQ`, `EXPECT_TRUE` expressives
- **GMock intégré** : Mocker les sockets UDP, filesystem (futur)
- **Test fixtures** : `SetUp()` / `TearDown()` pour initialiser registry ECS
- **Death tests** : Vérifier les assertions/crashes
- **CTest integration** : `enable_testing()` + `gtest_discover_tests()`

**Exemple R-TYPE** :
```cpp
TEST(RegistryTest, SpawnEntity) {
    registry reg;
    auto entity = reg.spawn_entity();
    EXPECT_GE(entity, 0);  // GTest assertion
}
```

**Pourquoi pas Catch2** : Syntaxe BDD non familière, compilation plus lente
**Pourquoi pas Doctest** : Moins de features (pas de mocking natif)
**Pourquoi pas Boost.Test** : Dépendance lourde, syntaxe verbeuse

### Outil de Coverage : lcov + gcov

**Justification** : lcov/gcov est l'outil standard pour la couverture de code C++ sur Unix.

**Comparatif** :

| Critère | gcov seul | **lcov + gcov** ✅ | Codecov online |
|---------|-----------|-------------------|----------------|
| Rapports HTML | ❌ | **✅** | ✅ |
| Ligne par ligne | ✅ | **✅** | ✅ |
| Branches | ✅ | **✅** | ✅ |
| Agrégation | Manuelle | **Automatique** | Automatique |
| Local | ✅ | **✅** | ❌ (cloud) |
| Gratuit | ✅ | **✅** | Limité (open-source) |

**Workflow R-TYPE** :
```bash
./r-type.sh coverage
# → Compile avec --coverage
# → Lance CTest
# → lcov capture les .gcda
# → genhtml crée build/coverage/index.html
```

**Avantages** :
- **Visuel** : Rapport HTML avec code source coloré (lignes vertes/rouges)
- **Gratuit** : Pas de limite, pas de compte requis
- **CI integration** : GitHub Actions upload vers Codecov

**Pourquoi pas Codecov seul** : Besoin de visualisation locale pour développement

---

## Qualité de Code

### Formateur de Code : clang-format

**Justification** : clang-format est le formateur C++ standard avec configuration fine.

**Comparatif** :

| Critère | Formatage manuel | **clang-format** ✅ | uncrustify |
|---------|------------------|---------------------|------------|
| Automatique | ❌ | **✅** | ✅ |
| Configuration | N/A | **Très fine (.clang-format)** | Complexe |
| Adoption | N/A | **Industrie** | Niche |
| IDE support | N/A | **Excellent** | Moyen |
| Vitesse | N/A | **Instantané** | Rapide |

**Configuration R-TYPE** :
```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
PointerAlignment: Left
```

**Avantages** :
- **Cohérence** : Tout le code formaté identiquement
- **Gain temps** : `./scripts/format.sh -f` = 1s pour tout formatter
- **CI enforcement** : PR bloquées si code mal formaté

**Pourquoi pas uncrustify** : Configuration obscure, moins de support IDE

### Configuration Style : Google C++ Style Guide (modifié)

**Justification** : Le Google Style est largement adopté avec des règles claires.

**Modifications R-TYPE** :
- **IndentWidth: 4** (au lieu de 2)
- **PointerAlignment: Left** (`int* ptr` vs `int *ptr`)
- **ColumnLimit: 100** (au lieu de 80)

**Alternatives considérées** :
- **LLVM Style** : Trop technique
- **Mozilla Style** : Moins populaire
- **Custom from scratch** : Trop de débats d'équipe

### Analyseur Statique : clang-tidy

**Justification** : clang-tidy détecte les bugs potentiels et applique les best practices modernes.

**Comparatif** :

| Critère | cppcheck | PVS-Studio | **clang-tidy** ✅ |
|---------|----------|------------|-------------------|
| Bugs détectés | Moyens | **Excellents** | Bons |
| Faux positifs | Peu | Peu | **Très peu** |
| C++20 support | Partiel | **Bon** | **Excellent** |
| Gratuit | ✅ | ❌ (payant) | **✅** |
| Intégration IDE | Moyenne | Bonne | **Excellente** |
| Modernize checks | Limité | Moyen | **Excellent** |

**Checks R-TYPE** :
- `modernize-*` : Préférer `auto`, `nullptr`, range-based loops
- `performance-*` : Optimisations (passer par référence, etc.)
- `bugprone-*` : Bugs classiques (use-after-move, etc.)

**Pourquoi pas cppcheck** : Moins de checks C++20
**Pourquoi pas PVS-Studio** : Payant pour projets commerciaux

---

## Documentation

### Générateur de Documentation : MkDocs + Material Theme

**Justification** : MkDocs produit des docs élégantes à partir de Markdown avec déploiement GitHub Pages intégré.

**Comparatif** :

| Critère | Doxygen | Sphinx | GitBook | **MkDocs + Material** ✅ |
|---------|---------|--------|---------|--------------------------|
| Syntaxe | Commentaires code | reStructuredText | Markdown | **Markdown** |
| Design moderne | Daté | Moyen | **Bon** | **Excellent** |
| GitHub Pages | Possible | Possible | Payant (cloud) | **Natif** |
| Navigation | Basique | Bonne | Excellente | **Excellente** |
| Search | Basique | Bonne | Bonne | **Instantanée** |
| Courbe apprentissage | Moyenne | Raide | Facile | **Facile** |
| API docs | **+++** | ++ | - | + |

**Avantages pour R-TYPE** :
- **Material theme** : Design responsive, dark mode, navigation fluide
- **Markdown natif** : Facile à écrire et maintenir (`docs/*.md`)
- **GitHub Pages deploy** : `mkdocs gh-deploy` = mise en ligne automatique
- **Search instantané** : Recherche full-text côté client
- **Navigation auto** : Génère la sidebar depuis `mkdocs.yml`

**Structure R-TYPE** :
```
docs/
├── index.md                    # Homepage
├── getting-started/            # Quickstart, building, installation
├── architecture/               # ECS, network, overview
└── developer-guide/            # Code style, contributing, technical choices
```

**Pourquoi pas Doxygen** : Idéal pour API reference, mais design daté pour docs projet
**Pourquoi pas Sphinx** : Syntax reStructuredText moins intuitive que Markdown
**Pourquoi pas GitBook** : Version gratuite limitée, pas d'auto-hébergement

### Format de Documentation : Markdown

**Justification** : Markdown est universellement supporté et facile à écrire.

**Avantages** :
- **GitHub-flavored** : Preview dans GitHub, MkDocs, VSCode
- **Lisible brut** : Pas besoin de compiler pour lire
- **Tooling** : Linters, formatters, extensions VSCode

### Hébergement Documentation : GitHub Pages

**Justification** : Gratuit, intégré à GitHub, déploiement automatique.

**Workflow R-TYPE** :
```bash
./scripts/docs.sh deploy
# → mkdocs build
# → Push vers branche gh-pages
# → Visible sur https://<user>.github.io/R-TYPE/
```

---

## CI/CD

### Plateforme CI/CD : GitHub Actions

**Justification** : GitHub Actions est intégré au repo avec runners gratuits pour projets open-source.

**Comparatif** :

| Critère | GitLab CI | Travis CI | CircleCI | **GitHub Actions** ✅ |
|---------|-----------|-----------|----------|----------------------|
| Intégration GitHub | Externe | Bonne | Bonne | **Native** |
| Gratuit (open-source) | ✅ | Limité | Limité | **Illimité** |
| Runners Linux/macOS/Win | ✅ | ✅ | ✅ | **✅** |
| Marketplace | Limité | Non | Limité | **Énorme** |
| Syntaxe | YAML | YAML | YAML | **YAML** |
| Matrix builds | ✅ | ✅ | ✅ | **✅** |

**Avantages pour R-TYPE** :
- **Pas de configuration externe** : Workflows dans `.github/workflows/`
- **Runners gratuits** : 20 jobs concurrents pour projets publics
- **Matrix strategy** : Tester Linux + macOS + Windows simultanément
- **Actions Marketplace** : `actions/checkout`, `actions/cache`, `codecov/codecov-action`

### Workflows Implémentés

**ci.yml** - Build multi-plateforme et tests :
- Matrice : Ubuntu 22.04, macOS-12, macOS-14 (ARM), Windows 2022
- Conan install + CMake build + CTest
- Artifact upload des binaires

**pr-check.yml** - Vérifications sur Pull Requests :
- clang-format check
- clang-tidy static analysis
- Build rapide (Linux only)

**coverage.yml** - Génération de rapports de couverture :
- Linux avec GCC + --coverage
- lcov + genhtml
- Upload vers Codecov

**Pourquoi pas GitLab CI** : Nécessite migration vers GitLab
**Pourquoi pas Travis CI** : Build minutes limitées (open-source récent)
**Pourquoi pas CircleCI** : Configuration plus complexe

---

## Plateformes Supportées

### Systèmes d'Exploitation : Linux, macOS, Windows

**Justification** : Support multiplateforme maximal pour développeurs et joueurs.

**Comparatif** :

| Plateforme | Compilateur | SFML support | Conan support |
|------------|-------------|--------------|---------------|
| **Linux** ✅ | GCC/Clang | **Excellent** | **Excellent** |
| **macOS** ✅ | Clang (Xcode) | **Excellent** | **Excellent** |
| **Windows** ✅ | MSVC/MinGW | **Excellent** | **Excellent** |

**Spécificités R-TYPE** :
- **Linux** : Plateforme dev principale, CI coverage
- **macOS** : Support Intel + Apple Silicon (M1/M2/M3)
- **Windows** : MSVC 2019+ requis, Conan MinGW possible

**Scripts cross-platform** : `r-type.sh` (bash) + `r-type.bat` (cmd)

### Compilateurs Supportés : GCC 10+, Clang 12+, MSVC 2019+

**Justification** : Support C++20 complet requis pour concepts et ranges.

**Versions minimales C++20** :
- **GCC 10+** : Premier GCC avec concepts complets
- **Clang 12+** : Support ranges et std::span
- **MSVC 2019+ (v16.11)** : C++20 complet à partir de Visual Studio 2019

---


**Dernière mise à jour** : 2025-11-26

