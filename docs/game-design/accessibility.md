# R-TYPE Accessibility Guide

R-TYPE is designed to be playable and enjoyable by the greatest number of people. This document outlines the accessibility features implemented in the game.

---

## Visual Accessibility

### Colorblind Filters

Real-time GLSL shader filters that transform colors for players with color vision deficiency.

**Available Modes:**
- **Protanopia** (Red-blind)
- **Deuteranopia** (Green-blind)
- **Tritanopia** (Blue-blind)
- **High Contrast** (Enhanced visibility)

**How to Enable:** Settings menu → Color Blind Mode → Select mode with arrow keys → Save

**What's Transformed:** All game elements (sprites, projectiles, UI, menus, effects) are transformed in real-time using scientifically accurate Brettel algorithms.

### Visual Feedback

Clear visual cues for all game events:
- Damage taken: Red screen flash
- Low health: Flashing health bar
- Power-ups: Card selection UI
- Boss appearance: Camera zoom + text
- Level transitions: Fade effects

---

## Input Accessibility

### Keyboard Navigation

Complete keyboard navigation throughout the entire game interface without requiring a mouse.

**Supported Areas:**
- Main menu (arrow keys + Enter)
- Lobby list and creation
- Settings menu
- In-game controls (arrows or WASD)
- Power-up selection (keys 1/2)

**Alternative Controls:** WASD scheme fully supported in addition to arrow keys.

**Auto-Fire Mode:** Toggle continuous automatic firing without holding the shoot button. Accessible in Settings menu for players with reduced mobility or hand strain.

**Planned Features:** Full key remapping, gamepad support, one-handed mode
- Settings (opens settings)
- Quit (exits with confirmation)

#### Lobby List

| Key | Action | Details |
|-----|--------|---------|
| **Up** | Previous lobby | Moves up in the list |
| **Down** | Next lobby | Moves down in the list |
| **Enter** | Join | Joins selected lobby |
**Planned Features:** Full key remapping, gamepad support, one-handed mode

---

## Gameplay Accessibility

### Friendly Fire Configuration

Lobbies allow players to enable or disable friendly fire damage between teammates.

**Default Mode (Disabled):**
- ✅ Players cannot hurt each other
- ✅ Encourages cooperative gameplay

**Enabled Mode:** Increases challenge by allowing player projectiles to damage allies.

**Exception:** Drones (Support Drone, Missile Drone) ✅ never damage their owner regardless of friendly fire setting.

### Difficulty Levels

Four difficulty options in lobby creation:
- **Easy:** -30% enemy health/damage, -20% spawn rate
- **Normal:** Balanced standard experience
- **Hard:** +50% health, +30% damage, +30% spawns
- **Impossible:** +100% health, +50% damage, +50% spawns

---

## Auditory Accessibility

### Sound Design

**Music Tracks:** 3 contextual tracks (menu, gameplay, boss battle)

**Sound Effects:** 11 distinct effects for actions (laser fire, explosions, hits, power-ups, shields, etc.)

**Visual Alternatives:** All critical audio cues have visual equivalents (enemy spawns, power-up drops, boss transformations, level transitions, game over screens)

**Planned:** Separate volume controls for Music, SFX, and UI sounds

---

## Settings Persistence

All accessibility settings are automatically saved to `settings.ini` and persist between game sessions.

**Settings Include:**
- ✅ Colorblind mode selection
- ✅ Volume levels (when implemented)
- ✅ Control schemes
- ✅ No reconfiguration needed on restart

---

## Future Roadmap

**Planned Features:**
- Subtitle system for audio events
- Customizable font size
- Motion reduction options
- Additional contrast presets
- Full control remapping
- Gamepad support with vibrations
- Screen reader support

---

## Feedback

We welcome accessibility feedback! Report issues or suggestions at [GitHub Issues](https://github.com/quent1111/R-TYPE/issues) with the `accessibility` label.

---

*Last updated: January 16, 2026*
