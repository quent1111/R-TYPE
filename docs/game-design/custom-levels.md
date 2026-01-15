# Custom Levels Guide

This guide explains how to create custom levels for R-Type using JSON configuration files.

## Table of Contents

1. [Getting Started](#getting-started)
2. [File Structure](#file-structure)
3. [Configuration Reference](#configuration-reference)
4. [Enemy Configuration](#enemy-configuration)
5. [Wave Configuration](#wave-configuration)
6. [Behavior Patterns](#behavior-patterns)
7. [Attack Patterns](#attack-patterns)
8. [Examples](#examples)

---

## Getting Started

Custom levels are JSON files placed in the `levels/custom/` directory. The game automatically loads all `.json` files from this directory at startup.

### Quick Start

1. Create a new `.json` file in `levels/custom/`
2. Define the basic structure with metadata, environment, enemies, and waves
3. Start the game - your level will appear in the level selection

### Minimal Example

```json
{
  "metadata": {
    "id": "my_first_level",
    "name": "My First Level",
    "author": "YourName",
    "version": "1.0.0",
    "description": "A simple custom level",
    "difficulty": 1
  },
  "environment": {
    "background_texture": "assets/bg.png",
    "scroll_speed": 50.0
  },
  "enemy_definitions": {
    "basic_enemy": {
      "id": "basic_enemy",
      "name": "Basic Enemy",
      "sprite": {
        "texture_path": "assets/r-typesheet26.png",
        "frame_width": 65,
        "frame_height": 50,
        "frame_count": 3,
        "frame_duration": 0.15,
        "scale_x": 1.5,
        "scale_y": 1.5
      },
      "health": 10,
      "speed": 100.0,
      "damage": 20,
      "score_value": 100
    }
  },
  "waves": [
    {
      "wave_number": 1,
      "enemies": [
        {
          "enemy_id": "basic_enemy",
          "count": 3,
          "spawn_delay": 1.0
        }
      ]
    }
  ]
}
```

---

## File Structure

A level configuration file has the following top-level structure:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `metadata` | object | Yes | Level information |
| `environment` | object | Yes | Background and audio settings |
| `enemy_definitions` | object | Yes | Enemy type definitions |
| `waves` | array | Yes | Wave configurations |
| `powerups` | array | No | Powerup spawn rules |
| `max_players` | integer | No | Maximum players (default: 4) |
| `time_limit` | float | No | Time limit in seconds |
| `lives` | integer | No | Starting lives (default: 3) |

---

## Configuration Reference

### Metadata

```json
{
  "metadata": {
    "id": "unique_level_id",
    "name": "Display Name",
    "author": "Author Name",
    "version": "1.0.0",
    "description": "Level description",
    "difficulty": 1,
    "preview_image": "assets/preview/level.png"
  }
}
```

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `id` | string | Yes | Unique identifier (no spaces) |
| `name` | string | Yes | Display name |
| `author` | string | No | Creator's name |
| `version` | string | No | Version string (default: "1.0.0") |
| `description` | string | No | Level description |
| `difficulty` | integer | No | 1-5 difficulty rating |
| `preview_image` | string | No | Preview image path |

### Environment

```json
{
  "environment": {
    "background_texture": "assets/bg.png",
    "parallax_layer_1": "assets/layer1.png",
    "parallax_layer_2": "assets/layer2.png",
    "scroll_speed": 50.0,
    "music": "assets/music/level.ogg",
    "ambient_sound": "assets/sounds/ambient.ogg"
  }
}
```

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `background_texture` | string | Yes | Main background image |
| `parallax_layer_1` | string | No | First parallax layer |
| `parallax_layer_2` | string | No | Second parallax layer |
| `scroll_speed` | float | No | Background scroll speed (default: 50.0) |
| `music` | string | No | Background music file |
| `ambient_sound` | string | No | Ambient sound file |

---

## Enemy Configuration

### Sprite Configuration

```json
{
  "sprite": {
    "texture_path": "assets/enemy.png",
    "frame_width": 64,
    "frame_height": 64,
    "frame_count": 4,
    "frame_duration": 0.1,
    "scale_x": 1.5,
    "scale_y": 1.5
  }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `texture_path` | string | "" | Path to sprite sheet |
| `frame_width` | integer | 32 | Width of each frame |
| `frame_height` | integer | 32 | Height of each frame |
| `frame_count` | integer | 1 | Number of animation frames |
| `frame_duration` | float | 0.1 | Seconds per frame |
| `scale_x` | float | 1.0 | Horizontal scale |
| `scale_y` | float | 1.0 | Vertical scale |

### Full Enemy Definition

```json
{
  "enemy_definitions": {
    "my_enemy": {
      "id": "my_enemy",
      "name": "My Enemy",
      "sprite": { ... },
      "health": 100,
      "speed": 150.0,
      "damage": 25,
      "score_value": 200,
      "behavior": { ... },
      "attack": { ... },
      "death_sound": "assets/sounds/death.ogg",
      "death_animation": { ... }
    }
  }
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `id` | string | - | Unique enemy identifier |
| `name` | string | "" | Display name |
| `sprite` | object | - | Sprite configuration |
| `health` | integer | 100 | Hit points |
| `speed` | float | 100.0 | Movement speed |
| `damage` | integer | 10 | Contact damage |
| `score_value` | integer | 100 | Points when killed |
| `behavior` | object | - | Movement behavior |
| `attack` | object | - | Attack pattern |
| `death_sound` | string | null | Death sound effect |
| `death_animation` | object | null | Death animation sprite |

---

## Wave Configuration

### Wave Structure

```json
{
  "waves": [
    {
      "wave_number": 1,
      "name": "Wave Name",
      "wave_delay": 2.0,
      "is_boss_wave": false,
      "trigger_condition": "all_enemies_dead",
      "music_override": "assets/music/boss.ogg",
      "enemies": [ ... ]
    }
  ]
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `wave_number` | integer | auto | Wave order number |
| `name` | string | "" | Wave display name |
| `wave_delay` | float | 2.0 | Delay before wave starts |
| `is_boss_wave` | boolean | false | Triggers boss spawn |
| `trigger_condition` | string | null | Condition to start wave |
| `music_override` | string | null | Override background music |
| `enemies` | array | [] | Enemy spawn configurations |

### Enemy Spawn Configuration

```json
{
  "enemies": [
    {
      "enemy_id": "basic_enemy",
      "count": 5,
      "spawn_delay": 0.5,
      "spawn_point": {
        "x": 1280,
        "y": 360,
        "position_type": "absolute",
        "offset_x": 0,
        "offset_y": 0
      },
      "formation": "v_shape"
    }
  ]
}
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `enemy_id` | string | - | Reference to enemy definition |
| `count` | integer | 1 | Number to spawn |
| `spawn_delay` | float | 0.5 | Delay between spawns |
| `spawn_point` | object | - | Spawn location config |
| `formation` | string | null | Formation pattern |

### Position Types

| Type | Description |
|------|-------------|
| `absolute` | Fixed x,y coordinates |
| `screen_right` | Right edge of screen + offset |
| `screen_top` | Top edge of screen + offset |
| `screen_bottom` | Bottom edge of screen + offset |
| `random_right` | Random Y on right edge |
| `random_top` | Random X on top edge |
| `random_bottom` | Random X on bottom edge |

### Formation Types

| Formation | Description |
|-----------|-------------|
| `line` | Horizontal line |
| `column` | Vertical column |
| `v_shape` | V formation |
| `diamond` | Diamond pattern |
| `circle` | Circular pattern |

---

## Behavior Patterns

### Behavior Configuration

```json
{
  "behavior": {
    "type": "sine_wave",
    "movement": {
      "type": "sine",
      "amplitude": 100.0,
      "frequency": 2.0,
      "phase": 0.0,
      "waypoints": [[100, 200], [300, 400]]
    },
    "tracks_player": false,
    "tracking_speed": 0.0,
    "aggro_range": 0.0
  }
}
```

### Behavior Types

| Type | Description |
|------|-------------|
| `straight` | Moves in straight line |
| `sine_wave` | Sinusoidal movement |
| `zigzag` | Zigzag pattern |
| `homing` | Tracks player |
| `stationary` | Does not move |
| `patrol` | Follows waypoints |
| `orbit` | Circular orbit |

### Movement Parameters

| Field | Type | Description |
|-------|------|-------------|
| `amplitude` | float | Wave amplitude (sine/zigzag) |
| `frequency` | float | Wave frequency |
| `phase` | float | Initial phase offset |
| `waypoints` | array | List of [x, y] points (patrol) |

---

## Attack Patterns

### Attack Configuration

```json
{
  "attack": {
    "type": "spread",
    "cooldown": 2.0,
    "burst_count": 3,
    "burst_delay": 0.1,
    "spread_angle": 30.0,
    "projectile_count": 5,
    "projectile": {
      "type": "basic",
      "sprite": { ... },
      "speed": 300.0,
      "damage": 15,
      "fire_rate": 1.0,
      "homing": false,
      "homing_strength": 0.0
    }
  }
}
```

### Attack Types

| Type | Description |
|------|-------------|
| `none` | Does not attack |
| `single` | Single projectile |
| `burst` | Multiple projectiles in sequence |
| `spread` | Fan of projectiles |
| `circular` | 360° projectile burst |
| `aimed` | Shoots toward player |
| `spawn` | Spawns enemies |

### Attack Parameters

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `cooldown` | float | 2.0 | Seconds between attacks |
| `burst_count` | integer | 1 | Projectiles per burst |
| `burst_delay` | float | 0.1 | Delay between burst shots |
| `spread_angle` | float | 0.0 | Total spread angle (degrees) |
| `projectile_count` | integer | 1 | Projectiles per shot |

### Projectile Configuration

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `type` | string | "basic" | Projectile type |
| `sprite` | object | - | Sprite configuration |
| `speed` | float | 400.0 | Projectile speed |
| `damage` | integer | 10 | Damage dealt |
| `homing` | boolean | false | Enable homing |
| `homing_strength` | float | 0.0 | Turn rate for homing |

---

## Powerup Configuration

```json
{
  "powerups": [
    {
      "powerup_type": "health",
      "spawn_chance": 0.15,
      "spawn_on_wave": 3,
      "spawn_condition": "score_above_1000"
    }
  ]
}
```

### Available Powerup Types

| Type | Description |
|------|-------------|
| `health` | Restores health |
| `attack` | Increases damage |
| `shield` | Temporary shield |
| `firerate` | Increases fire rate |
| `laser` | Laser weapon |
| `multishot` | Multiple projectiles |
| `missile_drone` | Drone companion |
| `canon` | Heavy cannon |

### Spawn Conditions

| Condition | Description |
|-----------|-------------|
| `score_above_X` | Score exceeds X |
| `health_below_X` | Health below X% |
| `wave_X` | Specific wave number |
| `random` | Random chance only |

---

## Examples

### Example: Sine Wave Enemy

```json
{
  "wave_enemy": {
    "id": "wave_enemy",
    "name": "Wave Fighter",
    "sprite": {
      "texture_path": "assets/r-typesheet24.png",
      "frame_width": 65,
      "frame_height": 66,
      "frame_count": 5,
      "frame_duration": 0.12,
      "scale_x": 1.5,
      "scale_y": 1.5
    },
    "health": 20,
    "speed": 120.0,
    "damage": 30,
    "score_value": 150,
    "behavior": {
      "type": "sine_wave",
      "movement": {
        "type": "sine",
        "amplitude": 100.0,
        "frequency": 2.0
      }
    },
    "attack": {
      "type": "single",
      "cooldown": 1.5,
      "projectile": {
        "speed": 250.0,
        "damage": 15
      }
    }
  }
}
```

### Example: Homing Enemy

```json
{
  "hunter": {
    "id": "hunter",
    "name": "Hunter Drone",
    "sprite": {
      "texture_path": "assets/drone.png",
      "frame_width": 32,
      "frame_height": 32,
      "frame_count": 4,
      "frame_duration": 0.08,
      "scale_x": 1.0,
      "scale_y": 1.0
    },
    "health": 5,
    "speed": 180.0,
    "damage": 20,
    "score_value": 75,
    "behavior": {
      "type": "homing",
      "tracks_player": true,
      "tracking_speed": 150.0,
      "aggro_range": 400.0
    },
    "attack": {
      "type": "none"
    }
  }
}
```

### Example: Boss Wave Trigger

```json
{
  "wave_number": 5,
  "name": "Boss Encounter",
  "wave_delay": 5.0,
  "is_boss_wave": true,
  "trigger_condition": "all_enemies_dead",
  "music_override": "assets/music/boss.ogg",
  "enemies": []
}
```

### Example: Complex Formation

```json
{
  "enemies": [
    {
      "enemy_id": "fighter",
      "count": 5,
      "spawn_delay": 0.3,
      "spawn_point": {
        "position_type": "screen_right",
        "offset_y": -100
      },
      "formation": "v_shape"
    },
    {
      "enemy_id": "bomber",
      "count": 2,
      "spawn_delay": 1.0,
      "spawn_point": {
        "position_type": "screen_right",
        "offset_y": 100
      },
      "formation": "line"
    }
  ]
}
```

---

## Best Practices

1. **Unique IDs**: Always use unique `id` values for levels and enemies
2. **Test Incrementally**: Start with simple waves and add complexity
3. **Balance Difficulty**: Adjust health, speed, and spawn counts together
4. **Use Formations**: Group enemies in formations for better gameplay
5. **Boss Waves**: Set `is_boss_wave: true` for boss encounters
6. **Validate JSON**: Use a JSON validator before testing

## Troubleshooting

### Common Issues

| Issue | Solution |
|-------|----------|
| Level not loading | Check JSON syntax validity |
| Enemies not spawning | Verify `enemy_id` matches definition |
| Textures missing | Check texture paths are correct |
| No animation | Ensure `frame_count` > 1 |

### Validation

The game logs warnings for:
- Missing required fields
- Invalid enemy references
- Empty wave definitions

Check the game logs for detailed error messages.

---

## File Locations

| Type | Path |
|------|------|
| Custom Levels | `levels/custom/*.json` |
| Built-in Levels | `levels/*.json` |
| Assets | `assets/` |
| Sounds | `assets/sounds/` |
| Music | `assets/music/` |
