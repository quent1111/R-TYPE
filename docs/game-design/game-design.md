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
- Accessibility: consider color-blind friendly palettes and configurable input bindings.

## Balancing Guidelines

- Health & Damage: keep HP low enough that mistakes matter, high enough to avoid frustration.
- Spawn Rates: gradual introduction of enemies; avoid overlapping high-density waves near player spawn.
- Power-ups: scale duration and potency so they feel meaningful but not game-breaking.
- Difficulty Curve: design levels/waves to teach mechanics early and introduce complexity steadily.

## Multiplayer & Network Considerations

- Authority Model: server authoritative for important state (HP, entity spawns, score). Clients perform prediction for movement and shooting to keep controls responsive.
- Replication: compress and send only deltas for entity state; prioritize player-owned entities and nearby threats.
- Lag Compensation: use client-side prediction for movement and client-side hit-simulation with server reconciliation.
- Bandwidth: tick rate around 20-30 updates/sec for world state; event-driven messages for spawns and deaths.

## Metrics & Telemetry

- Track: average player session length, death causes, most-used weapons, average time to clear wave.
- Use metrics for balancing: e.g., if many deaths are caused by a single hazard, adjust spawn/telegraph.

## Mapping Design → Codebase

- ECS Systems (see `engine/ecs`):
  - MovementSystem: applies velocity and input to player entities.
  - ProjectileSystem: spawns and advances projectiles, handles lifetime and collisions.
  - CollisionSystem: resolves collisions and applies damage.
  - SpawnSystem: schedules enemy waves and handles spawn patterns (data-driven via JSON/CSV).
  - PowerupSystem: applies power-up effects and manages durations.

- Key modules and files:
  - `server/game_logic/instances/`: server-side game loop and instance management for multiplayer matches.
  - `engine/core/`: shared systems like timing, resource management, and settings.
  - `client/ui/`: HUD elements (score, health, power-up icons) and input handling.

## Implementation Notes

- Data-driven wave definitions: store enemy wave patterns in external files so designers can iterate quickly without recompiling.
- Deterministic server simulation: keep authoritative server behavior deterministic wherever possible to simplify reconciliation.
- Modular systems: keep systems small and single-responsibility so it's easy to test and modify (e.g., separate movement from input handling).

## Design Checklist (for new features)

- [ ] Gameplay prototype implemented in a sandbox scene.
- [ ] Telemetry added to gather metrics for balancing.
- [ ] Visual/audio feedback implemented and tuned.
- [ ] Network behavior verified under artificial latency.
- [ ] Unit/integration tests added for critical systems.

## References

- See `docs/architecture/ecs.md` for details on the ECS structure.
- See `server/game_logic/` for server-side authoritative logic examples.


*Document last updated: 2025-12-01*
