# Player Guide

Welcome to R-TYPE! This guide will help you master the game, understand the controls, recognize enemies, and defeat bosses.

---

## Table of Contents

1. [Game Overview](#game-overview)
2. [Controls](#controls)
3. [How to Play](#how-to-play)
4. [Game Modes](#game-modes)
5. [Enemies](#enemies)
6. [Boss Fight](#boss-fight)
7. [Powerups & Weapons](#powerups--weapons)
8. [Tips & Strategies](#tips--strategies)

---

## Game Overview

R-TYPE is a classic side-scrolling shooter reimagined with modern multiplayer capabilities. Up to **4 players** can team up to fight through **5 challenging levels** filled with enemies, culminating in an epic boss battle.

**Key Features:**
- **4-player cooperative multiplayer** via network
- **5 progressive levels** with increasing difficulty
- **Epic boss battle** at level 5
- **Powerup system** with health restoration and weapon upgrades
- **Real-time combat** with smooth 60 FPS gameplay
- **Scoring system** with combo multipliers

---

## Controls

### Keyboard Controls

| Key | Action |
|-----|--------|
| **↑ Arrow** | Move Up |
| **↓ Arrow** | Move Down |
| **← Arrow** | Move Left |
| **→ Arrow** | Move Right |
| **Space** | Shoot |
| **ESC** | Pause / Return to Menu |

### Alternative Controls (WASD)

| Key | Action |
|-----|--------|
| **W** | Move Up |
| **S** | Move Down |
| **A** | Move Left |
| **D** | Move Right |
| **Space** | Shoot |

### Mouse Controls

- **Mouse Movement** - Control ship position (if enabled in settings)
- **Left Click** - Shoot
- **ESC** - Pause / Menu

---

## How to Play

### Starting a Game

1. **Launch the client**
   ```bash
   ./r-type.sh
   # or on Windows:
   r-type.bat
   ```

2. **Main Menu Options**
   - **Play** - Join a game server
   - **Settings** - Configure audio, controls, display
   - **Quit** - Exit the game

3. **Lobby System**
   - Enter server IP address (default: `localhost:12345`)
   - Wait for other players (supports 1-4 players)
   - Players can ready up
   - Game starts when all players are ready

### Gameplay Basics

- **Objective:** Survive through 5 levels and defeat the final boss
- **Health:** You start with **100 HP**
- **Lives:** Die = respawn at the start of the next level (if playing multiplayer)
- **Score:** Earn points by destroying enemies and maintaining combos
- **Powerups:** Collect powerups that appear after destroying enemies

### Level Progression

1. **Levels 1-4:** Waves of enemies appear continuously
2. **Level Completion:** Destroy all enemies to advance
3. **Level 5:** Face the **final boss**
4. **Victory:** Defeat the boss to win the game
5. **Game Over:** Occurs when all players are dead

---

## Game Modes

### Cooperative Multiplayer (1-4 Players)

- Play with friends over LAN or Internet
- Share the same game session
- Dead players respawn at the start of new levels
- Coordinate powerup selections
- Shared victory or defeat

### Server Information

- **Default Port:** `12345`
- **Protocol:** UDP
- **Max Players:** 4
- **Server Command:**
  ```bash
  ./r-type_server 0.0.0.0 12345
  ```

---

## Enemies

### Regular Enemies

#### Basic Enemy (Green)
- **HP:** 20
- **Damage:** 10 (contact)
- **Speed:** Medium
- **Behavior:** Moves straight from right to left
- **Score:** 100 points

#### Advanced Enemy (Red)
- **HP:** 50
- **Damage:** 20 (contact)
- **Speed:** Fast
- **Behavior:** Moves in wave patterns
- **Projectiles:** Shoots at players
- **Score:** 250 points

#### Homing Enemy (Yellow)
- **HP:** 50
- **Damage:** 10 (contact)
- **Speed:** 100 px/s
- **Behavior:** Tracks nearest player
- **Detection Range:** 200 pixels
- **Score:** 300 points
- **Note:** Spawned by the boss every 3 shots

### Enemy Projectiles

- **Damage:** 20 HP per hit
- **Speed:** 300 px/s
- **Color:** Red/Orange
- **Behavior:** Aimed at player position when fired

---

## Boss Fight

### Level 5 Boss

The final boss appears at **Level 5** and is the ultimate challenge.

#### Boss Statistics

| Attribute | Value |
|-----------|-------|
| **Health** | 2000 HP |
| **Damage (Contact)** | 50 HP |
| **Size** | Large (multiple hitboxes) |
| **Entry Position** | X=2400, Y=540 (off-screen right) |
| **Combat Position** | X=1500, Y=540 |
| **Entry Speed** | -150 px/s |

#### Boss Hitboxes

The boss has **3 hitbox zones**:

1. **Head** - 250×250 px (offset: -200, -350)
2. **Body** - 350×500 px (offset: 25, -100)
3. **Tail** - 200×200 px (offset: -100, 220)

All zones can be damaged independently.

#### Boss Behavior Phases

##### Phase 1: Entrance (0-2 seconds)
- Boss enters from the right side of the screen
- Moves to combat position (X=1500)
- **Strategy:** Position yourself and prepare for combat

##### Phase 2: Animation (2-4.5 seconds)
- Boss remains stationary for 2.5 seconds
- No attacks during this phase
- **Strategy:** Deal as much damage as possible

##### Phase 3: Combat (4.5 seconds+)
- **Projectile Attack:**
  - Fires aimed projectiles at **all living players**
  - Shoot cooldown: **1 second**
  - Projectile damage: **20 HP**
  - Projectile speed: **300 px/s**
  
- **Homing Enemy Spawn:**
  - Spawns a homing enemy every **3 shots**
  - Homing enemies track nearest player
  - **HP:** 50 | **Damage:** 10

#### Boss Fight Strategies

1. **Keep Moving:** The boss aims where you are, not where you'll be
2. **Focus Fire:** Concentrate damage on one hitbox zone
3. **Kill Homing Enemies:** Don't let them accumulate
4. **Use Powerups:** Save weapon upgrades for the boss
5. **Coordinate:** In multiplayer, have players focus different targets
6. **Stay Spread Out:** Reduces area damage from projectiles

#### Boss Defeat

- Boss is defeated when health reaches 0
- Level complete screen appears
- Victory!

---

## Powerups & Weapons

### Health Powerup

- **Effect:** Restores health
- **Amount:** +30 HP (configurable)
- **Appearance:** Red cross icon
- **Duration:** Instant

### Weapon Upgrade System

After completing certain levels, you can choose weapon upgrades:

#### Upgrade 1: Rapid Fire
- **Effect:** Increases fire rate by 50%
- **Cooldown:** 0.1s → 0.05s
- **Visual:** Blue weapon

#### Upgrade 2: Spread Shot
- **Effect:** Fires 3 projectiles in a spread pattern
- **Angles:** -15°, 0°, +15°
- **Damage:** Same per projectile
- **Visual:** Green weapon

#### Upgrade 3: Power Shot
- **Effect:** Doubles projectile damage
- **Damage:** 10 → 20
- **Fire Rate:** Slightly slower
- **Visual:** Red weapon

### Powerup Appearance

- Powerups spawn randomly when enemies are destroyed
- Float towards the bottom of the screen
- Disappear after 5 seconds if not collected
- **Collection:** Touch the powerup with your ship

---

## Tips & Strategies

### General Tips

1. **Stay Alert:** Enemies can appear from any direction
2. **Conserve Health:** Avoid unnecessary risks
3. **Learn Patterns:** Enemy movements are predictable
4. **Use the Edges:** Utilize screen edges for dodging
5. **Shoot Constantly:** No ammo limit, keep firing

### Multiplayer Coordination

1. **Spread Formation:** Don't cluster together
2. **Cover Each Other:** Watch for enemies targeting teammates
3. **Share Powerups:** Communicate who needs health most
4. **Focus Fire:** Take down dangerous enemies quickly
5. **Revive System:** Dead players respawn at next level

### Scoring System

- **Enemy Kill:** Base points (100-300)
- **Combo Multiplier:** Increases with consecutive kills
- **Max Combo:** 10x multiplier
- **Combo Timer:** 3 seconds between kills to maintain
- **Boss Bonus:** 5000 points for boss defeat

### Advanced Techniques

#### Dodging
- Learn enemy projectile speeds
- Use diagonal movement for maximum evasion
- Position yourself between projectiles

#### Weapon Management
- Save upgraded weapons for bosses
- Choose upgrades based on playstyle:
  - **Rapid Fire** - Beginners, consistent damage
  - **Spread Shot** - Crowd control
  - **Power Shot** - Boss damage, experienced players

#### Boss Optimization
- Memorize boss attack patterns
- Position near the center of the screen
- Focus on dodging over attacking when overwhelmed
- Use homing enemies as "warning signs" (every 3 shots)

---

## Troubleshooting

### Connection Issues

**Cannot connect to server:**
- Verify server is running: `./r-type_server 0.0.0.0 12345`
- Check firewall settings (allow UDP port 12345)
- Confirm IP address is correct
- Ping the server to test connectivity

**High Latency:**
- Use wired connection instead of WiFi
- Close bandwidth-intensive applications
- Choose a server geographically closer

### Gameplay Issues

**Low FPS:**
- Close background applications
- Reduce graphics settings
- Update graphics drivers

**Input Lag:**
- Check USB connection for keyboard/mouse
- Disable VSync if enabled
- Reduce network latency

---

## System Requirements

### Minimum Requirements

- **OS:** Windows 10, Linux (Ubuntu 20.04+), macOS 11+
- **CPU:** Intel Core i3 / AMD Ryzen 3
- **RAM:** 4 GB
- **GPU:** Integrated graphics (Intel HD 4000+)
- **Network:** Broadband Internet (for multiplayer)
- **Storage:** 500 MB

### Recommended Requirements

- **OS:** Windows 11, Linux (Ubuntu 22.04+), macOS 12+
- **CPU:** Intel Core i5 / AMD Ryzen 5
- **RAM:** 8 GB
- **GPU:** Dedicated GPU (GTX 1050+ / RX 560+)
- **Network:** Ethernet connection
- **Storage:** 1 GB

---

## Community & Support

### Reporting Bugs

Found a bug? Report it on our GitHub:
- **Repository:** [github.com/quent1111/R-TYPE](https://github.com/quent1111/R-TYPE)
- **Issues:** Create a new issue with reproduction steps
- **Pull Requests:** Contributions welcome!

### Getting Help

- **Documentation:** Check the [architecture docs](../architecture/overview.md)
- **Discord:** [Join our community](#) (if available)
- **Email:** Contact the development team

---

## Credits

**R-TYPE** is developed as a student project at Epitech.

- **Engine:** Custom ECS engine
- **Graphics:** SFML 2.6.1
- **Networking:** Asio 1.30.2
- **Build System:** CMake 3.20+ with Conan 2.x

**Special Thanks:**
- Original R-TYPE arcade game by Irem (1987)
- SFML community
- Epitech instructors and peers

---

**Good luck, pilot! The fate of the galaxy is in your hands. 🚀**
