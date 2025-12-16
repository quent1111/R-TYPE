# Client Refactoring R-TYPE

## Summary
The client has been refactored to separate responsibilities and reduce the "God Object" `Game.cpp` from **1011 lines → 503 lines**.

---

## New Structure

```
client/
├── include/
│   ├── common/           ← Shared utilities
│   │   ├── SafeQueue.hpp
│   │   └── Settings.hpp
│   ├── game/             ← Main game classes
│   │   ├── Entity.hpp
│   │   └── Game.hpp
│   ├── input/            ← Input management
│   │   ├── InputHandler.hpp
│   │   └── InputKey.hpp
│   ├── managers/         ← Resource singletons
│   │   ├── AudioManager.hpp
│   │   ├── EffectsManager.hpp
│   │   ├── FontManager.hpp
│   │   └── TextureManager.hpp
│   ├── network/          ← Network communication
│   │   ├── Messages.hpp
│   │   └── NetworkClient.hpp
│   ├── rendering/        ← Separate rendering
│   │   ├── GameRenderer.hpp
│   │   ├── HUDRenderer.hpp
│   │   └── OverlayRenderer.hpp
│   ├── states/           ← State machine
│   │   ├── IState.hpp
│   │   ├── StateManager.hpp
│   │   ├── MenuState.hpp
│   │   ├── LobbyState.hpp
│   │   └── GameState.hpp
│   └── ui/               ← UI components
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

##  Main Changes

### 1. Managers (Singleton Pattern)
```cpp
// Before: loading in Game.cpp
texture.loadFromFile("player.png");

// After: via centralized manager
auto& tex = managers::TextureManager::instance().load("player.png");
```

**Managers créés:**
- `TextureManager` - Texture management
- `FontManager` - Font management
- `AudioManager` - Sounds and music
- `EffectsManager` - Particles, screen shake, combos

### 2. Renderers (Display separation)
```cpp
// Before: everything in Game::render()
// After: delegated to specialized renderers
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
// Callbacks configured in Game
input_handler_.set_input_callback([this](uint8_t mask) {
    send_input_to_server(mask);
});
input_handler_.set_shoot_sound_callback([this]() {
    managers::AudioManager::instance().play_sound(SoundType::Shoot);
});
```

---

##  Design Patterns Used

| Pattern | Où | Pourquoi |
|---------|-----|----------|
| **Singleton** | Managers | Global access to resources |
| **Observer/Callback** | InputHandler | Decouple input from logic |
| **State** | StateManager | Menu → Lobby → Game |
| **Facade** | Game, Managers | Simplify APIs |
| **Flyweight** | TextureManager | Share textures |

---

##  Points of Attention

1. **Manager access:**
   ```cpp
   managers::TextureManager::instance().load("sprite.png");
   managers::AudioManager::instance().play_sound(SoundType::Hit);
   ```

2. **Convenience headers available:**
   ```cpp
   #include "managers/Managers.hpp"    // All managers
   #include "rendering/Rendering.hpp"  // All renderers
   ```

3. **Game.cpp remains the entry point** - It orchestrates components

---

##  Tested and Functional
- Build: 
- Tests: 4/4 pass
- Gameplay: Identical to before refactoring
