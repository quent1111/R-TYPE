# Quick Start

Get up and running with R-TYPE in **under 1 minute**!

## ⚡ Ultra-Fast Start

=== "Linux/macOS"

    ```bash
    # 1. Clone the repository
    git clone https://github.com/quent1111/R-TYPE.git
    cd R-TYPE
    
    # 2. Run server (everything auto-installs!)
    ./r-type.sh server
    ```

=== "Windows"

    ```cmd
    REM 1. Clone the repository
    git clone https://github.com/quent1111/R-TYPE.git
    cd R-TYPE
    
    REM 2. Run server (everything auto-installs!)
    r-type.bat server
    ```

!!! success "That's it!"
    The `r-type.sh` script automatically:
    
    - ✅ Installs Conan package manager
    - ✅ Downloads dependencies (SFML, Asio, GTest)
    - ✅ Builds the server
    - ✅ Launches it

---

## 🎮 Running the Game

### 1. Start the Server

=== "Quick Way (Recommended)"

    ```bash
    # Linux/macOS
    ./r-type.sh server
    
    # Windows
    r-type.bat server
    ```

=== "Manual Way"

    ```bash
    cd build
    ./bin/r-type_server
    ```

The server will start and listen for client connections on **port 4242** (default).

**Expected output:**
```
[Init] R-TYPE Server starting...
[Config] Port: 4242
[Core] Server initialized
[Core] Network loop started
[Core] Game loop started (60 Hz tick rate)
```

### 2. Start the Client

In a **new terminal**:

=== "Quick Way (Recommended)"

    ```bash
    # Linux/macOS
    ./r-type.sh client
    
    # Windows
    r-type.bat client
    ```

=== "Manual Way"

    ```bash
    cd build/build/Release/bin
    ./r-type_client
    ```

### 3. Play!

The client will automatically connect to `localhost:4242`.

**Main Menu:**
- Click **Play** to join the lobby
- Click **Ready** when ready to start
- Game begins when all players are ready (or after timeout)

**In-Game Controls:**
- Use **Arrow Keys** or **WASD** to move your spaceship
- Press **Space** or **Left Mouse** to shoot
- Press **1-2** to activate power-ups
- Press **ESC** to pause/exit

## Game Controls

| Key | Action |
|-----|--------|
| **↑ / W** | Move up |
| **← / A** | Move left |
| **↓ / S** | Move down |
| **→ / D** | Move right |
| **Space / Mouse1** | Shoot projectiles |
| **1** | Activate power-up slot 1 |
| **2** | Activate power-up slot 2 |
| **ESC** | Pause / Exit game |
| **P** | Pause (in-game) |

## Current Features

### ✅ Multiplayer Gameplay
- Up to 4 players cooperative
- Real-time UDP networking with entity synchronization
- Client-side interpolation for smooth movement
- Server-authoritative game logic

### ✅ Progressive Level System
- Multiple levels with increasing difficulty
- Enemy wave spawning system
- Boss encounters (Serpent, Compiler)
- Level progression tracking

### ✅ Combat System
- Player shooting with cooldown
- Enemy AI with different behaviors (basic, homing, flying)
- Collision detection (player-enemy, projectile-enemy)
- Health system with damage and death

### ✅ Power-Up System
- **Passive Power-ups**: Attack boost, fire rate, multi-shot, laser
- **Activable Power-ups**: Speed boost, shield, drone support, missile drone
- Power-up selection between levels
- Visual effects for power-up activation

### ✅ Visual & Audio
- Sprite-based graphics with animations
- Parallax scrolling backgrounds
- Particle effects (explosions, lasers)
- Screen shake on hits
- Sound effects and background music
- Damage flash effects

### ✅ UI & HUD
- Health bar display
- Score tracking with combo multiplier
- Level progress indicator
- Power-up status display
- Lobby system with ready status

## Playing with Friends (Multiplayer)

### Hosting a Server

```bash
# Start server on default port (4242)
./r-type.sh server

# Or specify custom port
./r-type.sh server -- --port 5000
```

### Connecting to Server

Clients can connect by:

1. Starting the client normally (connects to localhost:4242)
2. The client will show connection status in the main menu
3. Join lobby and wait for other players
4. Click "Ready" to start the game

**For LAN/Internet play:**
- Share your server's IP address with friends
- Ensure port 4242 (or custom port) is open in firewall
- Players may need to configure `settings.ini` to connect to your IP

Example `settings.ini` for client:
```ini
[Network]
server_address = 192.168.1.100
server_port = 4242
```

## Configuration

### Server Configuration

Edit `settings.ini` in the server directory:

```ini
[Server]
port = 4242
max_players = 4
tick_rate = 60
bind_address = 0.0.0.0

[Game]
difficulty = normal
starting_level = 1
```

### Client Configuration

Edit `settings.ini` in the client directory:

```ini
[Graphics]
resolution_width = 1920
resolution_height = 1080
fullscreen = false
vsync = true

[Audio]
master_volume = 0.8
music_volume = 0.7
sound_volume = 0.8
audio_enabled = true  # Set to false on Windows to disable audio

[Network]
server_address = localhost
server_port = 4242

[Input]
key_up = Up
key_down = Down
key_left = Left
key_right = Right
key_shoot = Space
```

**Note:** On Windows, audio is disabled by default due to SFML audio limitations. Set `audio_enabled = false` in the Audio section if you experience issues.

## Command Line Options

### Server Options

```bash
./r-type.sh server -- [options]

Options:
  --port PORT             Server port (default: 4242)
  --bind ADDRESS          Bind address (default: 0.0.0.0)
  --verbose               Enable verbose logging
```

**Examples:**
```bash
# Custom port
./r-type.sh server -- --port 5000

# Verbose logging
./r-type.sh server -- --verbose

# Bind to specific interface
./r-type.sh server -- --bind 192.168.1.100 --port 4242
```

### Client Options

The client reads configuration from `settings.ini`. You can also pass options:

```bash
# Run client with custom settings
./r-type.sh client

# The client will automatically connect to the server
# specified in settings.ini (localhost:4242 by default)
```

**Network Architecture:**
- Server uses dual-loop architecture (game loop + network loop)
- Game logic runs at 60 Hz tick rate
- Network loop handles async UDP I/O via Boost.Asio
- Responses are unicast to specific clients (no automatic broadcast)
- Client uses thread-safe queues for game/network communication

## Example Sessions

### Local Single Player (Testing)

```bash
# Terminal 1: Start server
./r-type.sh server

# Terminal 2: Start client
./r-type.sh client

# Play solo to test mechanics
```

### Local Multiplayer (2-4 Players)

```bash
# Terminal 1: Start server
./r-type.sh server

# Terminal 2: Player 1
./r-type.sh client

# Terminal 3: Player 2
./r-type.sh client

# Terminal 4: Player 3 (optional)
./r-type.sh client

# Terminal 5: Player 4 (optional)
./r-type.sh client

# All players join lobby, click Ready, and game starts!
```

### LAN Party

```bash
# Host machine (e.g., 192.168.1.100)
./r-type.sh server

# Guest machines - edit settings.ini:
# [Network]
# server_address = 192.168.1.100
# server_port = 4242

./r-type.sh client
```

### Internet Play

1. **Port forwarding** on router: Forward port 4242 to server machine
2. **Find public IP**: Check at [whatismyip.com](https://www.whatismyip.com/)
3. **Share IP** with friends
4. **Players configure** `settings.ini` with your public IP
5. **Firewall**: Ensure port 4242 is allowed

**Security Note:** Only host servers for trusted players.

## Next Steps

- 🏗️ [Architecture Overview](../architecture/overview.md) - Understand the ECS architecture
- 🌐 [Network Documentation](../architecture/network.md) - Learn about the network protocol
- 🎨 [Game Design](../game-design/gameplay.md) - Gameplay mechanics and features
- 🧪 [Testing Guide](../developer-guide/testing.md) - Run and write tests
- 🤝 [Contributing](../developer-guide/contributing.md) - Help improve R-TYPE

## Tips & Tricks

### Performance Optimization

- **Lower Resolution**: Set `resolution_width = 1280` and `resolution_height = 720` for better FPS
- **Disable VSync**: Set `vsync = false` for uncapped framerate
- **Fullscreen Mode**: Set `fullscreen = true` for better performance

### Network Optimization

- **Wired Connection**: Use Ethernet instead of Wi-Fi for stable connection
- **Low Latency**: Keep ping below 50ms for best experience
- **Firewall**: Ensure UDP port is open (default 4242)

### Gameplay Tips

- **Power-Up Priority**: Focus on fire rate and attack boosts early
- **Shield Usage**: Activate shield before boss encounters
- **Positioning**: Stay in the middle-left area for best reaction time
- **Combos**: Kill enemies quickly to build score multiplier
- **Boss Patterns**: Learn boss attack patterns to avoid damage

### Troubleshooting

#### Game Window Doesn't Open

```bash
# Check if SFML can create window
./r-type.sh client

# If it fails, check logs for errors
# Common issues: missing graphics drivers, X11 not available
```

#### Can't Connect to Server

1. **Check server is running**: Look for "Network loop started" message
2. **Verify address**: Ensure `settings.ini` has correct server address
3. **Check firewall**: Port 4242 must be open
4. **Test locally first**: Use `localhost` before trying LAN/Internet

#### Audio Not Working (Windows)

Audio is disabled by default on Windows. If you have SFML audio properly set up:

```ini
[Audio]
audio_enabled = true
```

#### Lag/Stuttering

- **Server**: Ensure server has stable 60 Hz tick rate
- **Client**: Check CPU usage, close other applications
- **Network**: Test with `ping` to check latency

#### Game Crashes

```bash
# Run with debug build for better error messages
./r-type.sh build --debug
./r-type.sh client

# Check logs in console for error details
```
