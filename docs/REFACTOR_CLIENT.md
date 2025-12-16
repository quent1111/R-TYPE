# 🎮 Refactoring Client R-TYPE

## Résumé
Le client a été refactoré pour séparer les responsabilités et réduire le "God Object" `Game.cpp` de **1011 lignes → 503 lignes**.

---

## 📁 Nouvelle Structure

```
client/
├── include/
│   ├── common/           ← Utilitaires partagés
│   │   ├── SafeQueue.hpp
│   │   └── Settings.hpp
│   ├── game/             ← Classes principales du jeu
│   │   ├── Entity.hpp
│   │   └── Game.hpp
│   ├── input/            ← Gestion des entrées
│   │   ├── InputHandler.hpp
│   │   └── InputKey.hpp
│   ├── managers/         ← Singletons de ressources
│   │   ├── AudioManager.hpp
│   │   ├── EffectsManager.hpp
│   │   ├── FontManager.hpp
│   │   └── TextureManager.hpp
│   ├── network/          ← Communication réseau
│   │   ├── Messages.hpp
│   │   └── NetworkClient.hpp
│   ├── rendering/        ← Affichage séparé
│   │   ├── GameRenderer.hpp
│   │   ├── HUDRenderer.hpp
│   │   └── OverlayRenderer.hpp
│   ├── states/           ← Machine à états
│   │   ├── IState.hpp
│   │   ├── StateManager.hpp
│   │   ├── MenuState.hpp
│   │   ├── LobbyState.hpp
│   │   └── GameState.hpp
│   └── ui/               ← Composants UI
│       ├── MenuComponents.hpp
│       └── SettingsPanel.hpp
└── src/
    ├── main_menu.cpp
    ├── game/Game.cpp
    ├── input/InputHandler.cpp
    ├── managers/*.cpp
    ├── network/NetworkClient.cpp
    ├── rendering/*.cpp
    ├── states/*.cpp
    └── ui/*.cpp
```

---

## 🔧 Changements Principaux

### 1. Managers (Singleton Pattern)
```cpp
// Avant: chargement dans Game.cpp
texture.loadFromFile("player.png");

// Après: via manager centralisé
auto& tex = managers::TextureManager::instance().load("player.png");
```

**Managers créés:**
- `TextureManager` - Gestion des textures
- `FontManager` - Gestion des polices
- `AudioManager` - Sons et musique
- `EffectsManager` - Particules, screen shake, combos

### 2. Renderers (Séparation d'affichage)
```cpp
// Avant: tout dans Game::render()
// Après: délégué à des renderers spécialisés
game_renderer_.render_background(window_);
game_renderer_.render_entities(window_, entities_, my_network_id_, dt);
hud_renderer_.render(window_, ...);
overlay_renderer_.render_game_over(window_, font_);
```

**Renderers créés:**
- `GameRenderer` - Background, entités, effets
- `HUDRenderer` - Score, timer, barre de vie, combo
- `OverlayRenderer` - Game over, level intro, powerups

### 3. InputHandler (Callback Pattern)
```cpp
// Callbacks configurés dans Game
input_handler_.set_input_callback([this](uint8_t mask) {
    send_input_to_server(mask);
});
input_handler_.set_shoot_sound_callback([this]() {
    managers::AudioManager::instance().play_sound(SoundType::Shoot);
});
```

---

## 📊 Design Patterns Utilisés

| Pattern | Où | Pourquoi |
|---------|-----|----------|
| **Singleton** | Managers | Accès global aux ressources |
| **Observer/Callback** | InputHandler | Découpler input de la logique |
| **State** | StateManager | Menu → Lobby → Game |
| **Facade** | Game, Managers | Simplifier les APIs |
| **Flyweight** | TextureManager | Partage des textures |

---

## ⚠️ Points d'Attention

1. **Accès aux managers:**
   ```cpp
   managers::TextureManager::instance().load("sprite.png");
   managers::AudioManager::instance().play_sound(SoundType::Hit);
   ```

2. **Headers de commodité disponibles:**
   ```cpp
   #include "managers/Managers.hpp"    // Tous les managers
   #include "rendering/Rendering.hpp"  // Tous les renderers
   ```

3. **Game.cpp reste le point d'entrée** - Il orchestre les composants

---

## ✅ Testé et Fonctionnel
- Build: ✅
- Tests: 4/4 passent
- Gameplay: Identique à avant le refactor
