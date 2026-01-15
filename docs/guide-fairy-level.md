# Guide de Création du Niveau "Monde des Fées"

## 📋 RÉCAPITULATIF COMPLET DES INSTRUCTIONS

### 1. ASSETS DISPONIBLES (dans assets/levels/fairy-world/)
| Fichier | Description | Dimensions | Frames | Notes |
|---------|-------------|------------|--------|-------|
| `fairy-bg.png` | Fond du niveau | 1536x1024 | 1 | Statique |
| `fairy1.png` | Ennemi Pink Fairy | 237x74 | 3 (79x74) | Mirror X, attack: front |
| `fairy2.png` | Ennemi Blue Fairy | 423x75 | 7 (60x75) | Mirror X, attack: targeted |
| `fairy3.png` | Ennemi Green Fairy | 269x76 | 3 (89x76) | Mirror X, attack: spread |
| `fairy-projectiles.png` | Projectiles ennemis | 28x22 | 2 (14x22) | Scale x1.5 |
| `unicornboss.png` | Boss Licorne | 187x73 | 2 (93x73) | Mirror X |
| `fairy-missiles.png` | Missiles du boss | 41x35 | 2 (20x35) | Rotation 90° |

### 2. CONFIGURATION DES SPRITES ✅
- [x] Tableau de frames pour animations
- [x] Modification de la taille du sprite (scale)
- [x] Rotation du sprite (angle)
- [x] Mirror horizontal/vertical

### 3. TYPES D'ATTAQUES PRÉDÉFINIES ✅
| Type | Description | Assignation |
|------|-------------|-------------|
| `front` | Tir simple devant | Fairy 1 (Pink) |
| `targeted` | Ciblé sur le joueur | Fairy 2 (Blue) |
| `spread` | Tir en éventail | Fairy 3 (Green) |

### 4. CONFIGURATION DES PROJECTILES ✅
- Sprite/animation custom (fairy-projectiles.png)
- Scale 1.5x
- Animation 2 frames

### 5. BOSS (après 5 vagues) ✅
- Sprite: unicornboss.png (2 frames, 93x73)
- Projectiles: fairy-missiles.png (rotation 90°)
- Attaque: spread avec 7 projectiles

### 6. BACKGROUND ✅
- fairy-bg.png en mode statique (background_static: true)
- scroll_infinite: false

---

## ✅ TO-DO LIST - ÉTAT ACTUEL

### ÉTAPE 1: Organisation des Assets ✅ COMPLÉTÉE
- [x] Créer dossier `assets/levels/fairy-world/`
- [x] Déplacer tous les assets de temp-assets vers ce dossier
- [x] Vérifier que tous les fichiers sont présents

### ÉTAPE 2: Mise à jour du Schéma LevelConfig ✅ COMPLÉTÉE
- [x] Ajouter `mirror_x` et `mirror_y` à SpriteConfig
- [x] Ajouter `rotation` (angle en degrés) à SpriteConfig
- [x] Ajouter `scroll_infinite` à EnvironmentConfig
- [x] Ajouter `background_static` à EnvironmentConfig
- [x] Ajouter `aim_at_player` à AttackPatternConfig

### ÉTAPE 3: Mise à jour du Parser JSON ✅ COMPLÉTÉE
- [x] Parser les nouveaux champs sprite (mirror, rotation)
- [x] Parser scroll_infinite pour background
- [x] Parser background_static pour background
- [x] Parser aim_at_player pour les attaques

### ÉTAPE 4: Mise à jour des Composants ✅ COMPLÉTÉE
- [x] Ajouter mirror_x, mirror_y, rotation à sprite_component
- [x] Créer custom_attack_config component
- [x] Créer boss_tag component
- [x] Enregistrer les nouveaux composants dans GameSession

### ÉTAPE 5: Mise à jour du Custom Wave System ✅ COMPLÉTÉE
- [x] Implémenter spawnCustomEnemy avec support mirror/rotation
- [x] Implémenter spawnCustomBoss
- [x] Gérer les vagues de boss

### ÉTAPE 6: Création du Fichier JSON ✅ COMPLÉTÉE
- [x] Metadata (id: fairy_world)
- [x] Environment (background statique)
- [x] Enemy definitions (fairy1, fairy2, fairy3)
- [x] Boss definition (unicorn_boss)
- [x] Projectile configs avec animations
- [x] 5 vagues de fairies + 1 vague boss
- [x] Powerups configurés

### ÉTAPE 7: Client Integration ✅ COMPLÉTÉE
- [x] Ajouter "Fairy World" à la liste des niveaux
- [x] Charger les textures fairy-world dans Game.cpp

### ÉTAPE 8: Tests et Validation 🔄 EN COURS
- [ ] Compiler le projet
- [ ] Tester le chargement du niveau
- [ ] Vérifier les animations
- [ ] Vérifier les attaques
- [ ] Vérifier le boss

---

## 📊 FICHIERS MODIFIÉS

1. `game-lib/include/level/LevelConfig.hpp` - Ajout mirror, rotation, scroll options
2. `game-lib/src/level/LevelConfigParser.cpp` - Parsing des nouvelles options
3. `game-lib/include/components/game_components.hpp` - Nouveaux composants
4. `game-lib/src/systems/custom_wave_system.cpp` - Spawn avec mirror/rotation
5. `game-lib/include/systems/custom_wave_system.hpp` - Déclaration spawnCustomBoss
6. `server/src/game/GameSession.cpp` - Enregistrement composants + level ID 4
7. `client/src/states/LobbyState.cpp` - Ajout "Fairy World" à la liste
8. `client/src/game/Game.cpp` - Chargement textures fairy-world
9. `levels/custom/fairy_world.json` - Fichier de configuration complet

---

## ⚠️ NOTE IMPORTANTE

Le système actuel utilise des types d'entités hardcodés côté client (0x02 = Enemy).
Les ennemis custom apparaîtront avec le sprite standard tant que le système de
transmission de sprites réseau n'est pas implémenté.

Pour que les sprites custom apparaissent correctement, il faudra:
1. Soit modifier le protocole réseau pour transmettre le sprite path
2. Soit utiliser un système de mapping level->sprites côté client
3. Soit ajouter de nouveaux EntityTypes pour les différents types d'ennemis custom

