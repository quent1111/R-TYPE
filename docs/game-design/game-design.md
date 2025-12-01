# Game Design — R-TYPE

This document describes the high-level game design for R-TYPE, including core mechanics, entities, player experience goals, balancing guidance, and notes on mapping these systems to the codebase.

## High-Level Objective

- Fast-paced 2D shoot 'em up with cooperative multiplayer support.
- Emphasize responsive controls, predictable enemy behavior, and clear feedback for player actions.
- Support both local and networked play while minimizing perceived latency for critical interactions.

## Core Mechanics

- Player movement: 2D plane movement with inertia tuned for snappy control.
- Shooting: continuous fire and/or charged shots depending on weapon/power-up.
- Enemy waves: patterned spawns with increasing complexity and telegraphed attacks.
- Collisions: instantaneous hit detection for bullets vs. entities; friendly-fire configurable.
- Power-ups: temporary upgrades (fire rate, spread, shield) and persistent pickups (extra life).
- Scoring: points for enemy destruction, combos for multiple kills, and bonuses for objectives.

## Entity Types & Roles

- Player: controlled entity, receives input, fires projectiles, has HP and lives.
- Enemy (Grunt): basic target with simple movement patterns and weak attacks.
- Enemy (Elite/Boss): stronger enemies with multiple phases and attack patterns.
- Projectile: bullets and missiles spawned by players or enemies; have speed, damage, lifetime.
- Power-up / Pickup: items that modify player state when collected.
- Hazard: environmental obstacles (e.g., mines, lasers) that damage or modify movement.

## Player Feel & UX Goals

- Controls: minimal input lag; target 60 FPS with <100ms round-trip in networked play.
- Feedback: clear audio-visual cues for hits, damage taken, power-up collection, and deaths.
- Readability: distinct sprite silhouettes and color palette to separate players, enemies, projectiles, and effects.
- Responsiveness: controls must feel tight and predictable to maintain player confidence.

## Accessibility & Inclusive Design

R-TYPE is designed to be playable and enjoyable by the widest possible audience. Accessibility is not an afterthought—it's a core design principle that enhances the experience for all players.

### Visual Accessibility

#### Color Blindness Support
- Palette Design: Primary UI and gameplay elements use color combinations that are distinguishable across all types of color vision deficiency (Protanopia, Deuteranopia, Tritanopia).
- Color Blind Modes: Provide preset color schemes optimized for different types of color blindness:
  - Protanopia mode (red-green, red appears darker)
  - Deuteranopia mode (red-green, green appears darker)
  - Tritanopia mode (blue-yellow)
  - High contrast mode for low vision players
- Projectile Clarity: Player bullets vs. enemy bullets use different shapes (e.g., circles vs. diamonds) and border styles.

#### Visual Clarity
- Contrast Ratios: UI text and critical gameplay elements maintain WCAG AAA contrast ratios (minimum 7:1) against backgrounds.
- Particle Effect Reduction: Option to reduce or disable non-essential visual effects (screen shake, background particles) to minimize visual noise.

### Auditory Accessibility

#### Sound Design
- Visual Subtitles for Audio Cues: Important audio events (enemy spawn warnings, power-up drops, boss phase changes) have optional on-screen text indicators.
- Volume Mixing: Separate volume controls for:
  - Music
  - Sound Effects (SFX)
  - UI sounds

### Motor/Input Accessibility

#### Control Customization
- Full Key Remapping: Every action (movement, shoot, special, pause, etc.) can be rebound to any key/button.
- Multiple Control Schemes: Ship with preset configurations:
  - Classic (arrow keys + Z/X)
  - WASD + Mouse
  - Gamepad (with multiple layouts)
  - One-handed mode (all actions accessible with one hand)
- Auto-Fire Mode: Hold-to-shoot can be toggled to auto-fire (tap once to start/stop).

## Balancing Guidelines

- Health & Damage: keep HP low enough that mistakes matter, high enough to avoid frustration.
- Spawn Rates: gradual introduction of enemies; avoid overlapping high-density waves near player spawn.
- Power-ups: scale duration and potency so they feel meaningful but not game-breaking.
- Difficulty Curve: design levels/waves to teach mechanics early and introduce complexity steadily.

*Document last updated: 2025-12-01*
