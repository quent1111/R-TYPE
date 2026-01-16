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

## Accessibility

R-TYPE includes comprehensive accessibility features to ensure the game is playable by everyone. For complete details on:

- **Color blindness filters** (GLSL shaders with Protanopia, Deuteranopia, Tritanopia, High Contrast modes)
- **Keyboard navigation** (full arrow key support in menus and settings)
- **Control schemes** (arrow keys, WASD alternatives)
- **Gameplay options** (friendly fire configuration, difficulty settings)
- **Visual and auditory accommodations**

See the dedicated **[Accessibility Guide](accessibility.md)** for full documentation.

## Balancing Guidelines

- Health & Damage: keep HP low enough that mistakes matter, high enough to avoid frustration.
- Spawn Rates: gradual introduction of enemies; avoid overlapping high-density waves near player spawn.
- Power-ups: scale duration and potency so they feel meaningful but not game-breaking.
- Difficulty Curve: design levels/waves to teach mechanics early and introduce complexity steadily.

*Document last updated: 2025-12-01*
