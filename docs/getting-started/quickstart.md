# Quick Start

Get up and running with R-TYPE in minutes!

## Running the Game

### 1. Start the Server

```bash
cd build
./r-type_server
```

The server will start and listen for client connections on the default port (usually 4242).

### 2. Start the Client

In a new terminal:

```bash
cd build
./r-type_client
```

### 3. Connect and Play!

- Enter the server IP address (use `localhost` for local testing)
- Click "Connect"
- Use arrow keys to move and space to shoot

## Game Controls

| Key | Action |
|-----|--------|
| ↑ ↓ ← → | Move spaceship |
| Space | Shoot |
| ESC | Pause / Menu |

## Playing with Friends

### Host a Server

```bash
./r-type_server --port 4242
```

Share your public IP address with friends.

### Join a Server

```bash
./r-type_client --connect <server-ip>:4242
```

## Configuration

### Server Configuration

Edit `server.cfg`:

```ini
[Server]
port = 4242
max_players = 4
tick_rate = 60

[Game]
difficulty = normal
friendly_fire = false
```

### Client Configuration

Edit `client.cfg`:

```ini
[Graphics]
resolution = 1920x1080
fullscreen = false
vsync = true

[Audio]
master_volume = 80
music_volume = 70
sfx_volume = 80

[Network]
default_server = localhost
default_port = 4242
```

## Command Line Options

### Server Options

```bash
./r-type_server [options]

Options:
  -p, --port PORT         Server port (default: 4242)
  -m, --max-players NUM   Maximum players (default: 4)
  -t, --tick-rate RATE    Game tick rate (default: 60)
  -c, --config FILE       Config file path
  -v, --verbose           Verbose logging
  -h, --help              Show help message
```

### Client Options

```bash
./r-type_client [options]

Options:
  -c, --connect ADDRESS   Server address
  -p, --port PORT         Server port (default: 4242)
  -f, --fullscreen        Start in fullscreen
  -w, --windowed          Start in windowed mode
  -r, --resolution WxH    Window resolution
  -v, --verbose           Verbose logging
  -h, --help              Show help message
```

## Example Sessions

### Local Multiplayer

```bash
# Terminal 1: Start server
./r-type_server -v

# Terminal 2: Client 1
./r-type_client -c localhost

# Terminal 3: Client 2
./r-type_client -c localhost
```

### LAN Party

```bash
# Host machine (192.168.1.100)
./r-type_server -p 4242 -m 8

# Guest machines
./r-type_client -c 192.168.1.100:4242
```

## Next Steps

- 🏗️ [Architecture Overview](../architecture/overview.md) - Understand how R-TYPE works
- 🎨 [Customization Guide](../guides/customization.md) - Customize your game
- 🤝 [Contributing](../developer-guide/contributing.md) - Help improve R-TYPE

## Tips & Tricks

!!! tip "Performance"
    For better performance on lower-end machines, reduce the resolution and disable vsync.

!!! warning "Firewall"
    Make sure port 4242 (or your chosen port) is open in your firewall for multiplayer.

!!! info "Network"
    For best multiplayer experience, use a wired connection and ensure low latency to the server.
