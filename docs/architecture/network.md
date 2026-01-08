# Network Architecture

R-TYPE implements a **client-server architecture** with **UDP-based networking** for real-time multiplayer gameplay. The design separates network I/O from game logic using a **dual-loop architecture**.

##  Key Design Principles

### 1. Server Authority

The server is the **single source of truth**:
-  All game logic runs on the server
-  Collision detection on the server
-  Entity spawning controlled by server
-  Clients receive state updates and render

**Why?** Prevents cheating and ensures consistent gameplay across all clients.

### 2. Dual-Loop Architecture

```
┌─────────────────────────────────────┐
│       Game Loop (Main Thread)       │
│   - Fixed 60Hz tick rate            │
│   - Deterministic game logic        │
│   - Reads from input_queue_         │
│   - Writes to output_queue_         │
└─────────────────────────────────────┘
              ↕
   ThreadSafeQueue<NetworkPacket>
              ↕
┌─────────────────────────────────────┐
│      Network Loop (ASIO Thread)     │
│   - Asynchronous UDP I/O            │
│   - Non-blocking packet reception   │
│   - Sends queued packets            │
└─────────────────────────────────────┘
```

**Benefits:**
-  Game logic never blocks on I/O
-  Network thread handles async operations
-  Thread-safe communication via queues
-  Deterministic game simulation

### 3. Binary Protocol

Uses efficient binary packets instead of JSON/XML:
-  Minimal bandwidth usage
-  Fast serialization/deserialization
-  Type-safe with opcodes
-  Little-endian encoding (x86/x64 standard)

##  Network Components

### Server: UDPServer

Located in `server/include/network/UDPServer.hpp`

```cpp
namespace server {

class UDPServer {
public:
    // Constructor
    UDPServer(asio::io_context& io_context, 
              const std::string& bind_address, 
              unsigned short port);
    
    // Game loop operations (called from main thread)
    bool get_input_packet(NetworkPacket& packet);
    void queue_output_packet(const NetworkPacket& packet);
    void process_output_queue();
    
    // Direct send operations
    void send_to_endpoint(const asio::ip::udp::endpoint& endpoint, 
                          const std::vector<uint8_t>& data);
    void send_to_client(int client_id, const std::vector<uint8_t>& data);
    
    // Lifecycle
    void stop();
    
private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    
    // Thread-safe queues
    ThreadSafeQueue<NetworkPacket> input_queue_;
    ThreadSafeQueue<NetworkPacket> output_queue_;
    
    // Client tracking
    std::unordered_map<int, ClientEndpoint> clients_;
    std::mutex clients_mutex_;
    
    // Async receive
    void start_receive();
    void handle_receive(const asio::error_code& error, size_t bytes_transferred);
};

} // namespace server
```

**Key Features:**
- **Async Receive**: `start_receive()` + `handle_receive()` chain
- **Client Tracking**: Maps client ID → UDP endpoint
- **Thread Safety**: Mutexes protect shared state
- **Queue-Based**: No blocking I/O in game loop

**Usage Example:**

```cpp
// In server main thread
asio::io_context io_context;
server::UDPServer server(io_context, "0.0.0.0", 12345);

// Network thread
std::thread network_thread([&io_context]() {
    io_context.run();  // Run async I/O loop
});

// Game loop (main thread)
while (running) {
    // 1. Receive input from clients
    NetworkPacket packet;
    while (server.get_input_packet(packet)) {
        handle_client_input(packet);
    }
    
    // 2. Update game logic
    update_game_logic(dt);
    
    // 3. Broadcast state to clients
    broadcast_entity_positions(server);
    
    // 4. Send queued packets
    server.process_output_queue();
    
    // 5. Fixed timestep
    std::this_thread::sleep_for(16ms);  // 60 Hz
}

server.stop();
network_thread.join();
```

### Client: NetworkClient

Located in `client/include/network/NetworkClient.hpp`

```cpp
class NetworkClient {
public:
    explicit NetworkClient(asio::io_context& io_context);
    
    // Connection management
    void connect(const std::string& server_address, const std::string& port);
    void disconnect();
    bool is_connected() const;
    
    // Send/Receive
    void send(const std::vector<uint8_t>& data);
    bool try_pop_message(std::vector<uint8_t>& out_message);
    
private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    
    // Thread-safe queue for received messages
    ThreadSafeQueue<std::vector<uint8_t>> incoming_messages_;
    
    std::atomic<bool> connected_{false};
    
    // Async operations
    void start_receive();
    void handle_receive(const asio::error_code& error, size_t bytes_transferred);
};
```

**Usage Example:**

```cpp
// In client
asio::io_context io_context;
NetworkClient network_client(io_context);

// Connect to server
network_client.connect("127.0.0.1", "12345");

// Network thread
std::thread network_thread([&io_context]() {
    io_context.run();
});

// Game loop
while (running) {
    // 1. Send input to server
    std::vector<uint8_t> input_packet = create_input_packet(input_mask);
    network_client.send(input_packet);
    
    // 2. Receive server updates
    std::vector<uint8_t> message;
    while (network_client.try_pop_message(message)) {
        process_server_message(message);
    }
    
    // 3. Update local state
    update_client(dt);
    
    // 4. Render
    render();
}

network_client.disconnect();
network_thread.join();
```

### NetworkPacket Structure

```cpp
// server/include/common/NetworkPacket.hpp
struct NetworkPacket {
    asio::ip::udp::endpoint sender;  // Who sent this
    std::vector<uint8_t> data;       // Packet payload
    
    NetworkPacket() = default;
    NetworkPacket(const asio::ip::udp::endpoint& ep, std::vector<uint8_t> d)
        : sender(ep), data(std::move(d)) {}
};
```

**Why include sender?**
- Server needs to know who to reply to (unicast)
- Enables per-client tracking and responses
- Prevents broadcast storms

##  Protocol Specification

### Packet Format

Every packet starts with a header:

```cpp
struct PacketHeader {
    uint8_t opcode;        // Message type
    uint16_t payload_size; // Bytes after header
};
```

**Total packet structure:**
```
[1 byte: Opcode][2 bytes: Payload Size][N bytes: Payload]
```

### Byte-Level Format Examples

This section provides detailed byte-by-byte breakdowns of actual network packets to help understand the binary protocol format.

#### Example 1: JOIN_LOBBY Packet

**Description:** Client sends this packet to join the game lobby (no payload).

**Byte Layout:**
```
Offset   0    1    2
       ┌────┬────┬────┐
       │ 01 │ 00 │ 00 │
       └────┴────┴────┘
         │    │    │
         │    └────┴───── Payload Size (uint16_t, little-endian) = 0
         │
         └──────────────── Opcode = 0x01 (JOIN_LOBBY)

Total Size: 3 bytes
```

**Hexadecimal:** `01 00 00`

**C++ Serialization:**
```cpp
std::vector<uint8_t> packet(3);
packet[0] = 0x01;  // Opcode: JOIN_LOBBY
packet[1] = 0x00;  // Payload size low byte
packet[2] = 0x00;  // Payload size high byte
network_client.send(packet);
```

---

#### Example 2: PLAYER_INPUT Packet

**Description:** Client sends player input (movement + shoot).

**Scenario:** Player pressing UP + RIGHT + SHOOT

**Byte Layout:**
```
Offset   0    1    2    3
       ┌────┬────┬────┬────┐
       │ 10 │ 01 │ 00 │ 19 │
       └────┴────┴────┴────┘
         │    │    │    │
         │    │    │    └──── Input Mask (uint8_t) = 0x19 = 0b00011001
         │    │    │          Bits: SHOOT(1) | RIGHT(1) | UP(1)
         │    └────┴───────── Payload Size (uint16_t) = 1
         │
         └──────────────────── Opcode = 0x10 (PLAYER_INPUT)

Total Size: 4 bytes
```

**Input Mask Bit Mapping:**
```
Bit:    7   6   5   4   3   2   1   0
       ┌───┬───┬───┬───┬───┬───┬───┬───┐
       │ 0 │ 0 │ 0 │ 1 │ 1 │ 0 │ 0 │ 1 │  = 0x19
       └───┴───┴───┴───┴───┴───┴───┴───┘
         │   │   │   │   │   │   │   │
         │   │   │   │   │   │   │   └──── UP    (0x01)
         │   │   │   │   │   │   └──────── DOWN  (0x02)
         │   │   │   │   │   └──────────── LEFT  (0x04)
         │   │   │   │   └──────────────── RIGHT (0x08)
         │   │   │   └──────────────────── SHOOT (0x10)
         └───┴───┴───────────────────────── Reserved
```

**Hexadecimal:** `10 01 00 19`

**C++ Serialization:**
```cpp
uint8_t input_mask = INPUT_UP | INPUT_RIGHT | INPUT_SHOOT;  // 0x01 | 0x08 | 0x10 = 0x19
std::vector<uint8_t> packet = {
    0x10,  // Opcode: PLAYER_INPUT
    0x01,  // Payload size low byte (1 byte)
    0x00,  // Payload size high byte
    input_mask  // Input data
};
network_client.send(packet);
```

---

#### Example 3: ENTITY_POSITIONS Packet (2 entities)

**Description:** Server broadcasts positions of 2 entities (1 player, 1 enemy).

**Scenario:**
- **Entity 1** (Player): ID=42, Position=(100.5, 200.0), Velocity=(5.0, 0.0), Type=0x01
- **Entity 2** (Enemy): ID=100, Position=(400.0, 150.5), Velocity=(-3.0, 1.0), Type=0x02

**Byte Layout:**
```
Offset   0    1    2    3    4    5    6    7    8    9   10   11   12 ...
       ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬───
       │ 11 │ 2A │ 00 │ 02 │ 00 │ 00 │ 00 │ 2A │ 00 │ 00 │ 00 │ 00 │C8 │...
       └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴───
         │    │    │    │    └────┴────┴────┘
         │    │    │    │         │
         │    │    │    │         └──────────── Entity Count (uint32_t) = 2
         │    └────┴────┘
         │         │
         │         └────────────────────────── Payload Size (uint16_t) = 42
         │
         └──────────────────────────────────── Opcode = 0x11 (ENTITY_POSITIONS)

Entity 1 Data (bytes 7-27):
  7-10: Network ID (uint32_t, LE) = 42        → [2A 00 00 00]
 11-14: Position X (float, LE) = 100.5        → [00 C8 42 42]
 15-18: Position Y (float, LE) = 200.0        → [00 00 48 43]
 19-22: Velocity X (float, LE) = 5.0          → [00 00 A0 40]
 23-26: Velocity Y (float, LE) = 0.0          → [00 00 00 00]
    27: Entity Type (uint8_t) = 0x01          → [01]

Entity 2 Data (bytes 28-48):
 28-31: Network ID (uint32_t, LE) = 100       → [64 00 00 00]
 32-35: Position X (float, LE) = 400.0        → [00 00 C8 43]
 36-39: Position Y (float, LE) = 150.5        → [00 40 16 43]
 40-43: Velocity X (float, LE) = -3.0         → [00 00 40 C0]
 44-47: Velocity Y (float, LE) = 1.0          → [00 00 80 3F]
    48: Entity Type (uint8_t) = 0x02          → [02]

Total Size: 49 bytes (3 header + 4 count + 21*2 entities)
```

**Hexadecimal (first entity only):**
```
11 2A 00 02 00 00 00 2A 00 00 00 00 C8 42 42 00 00 48 43 00 00 A0 40 00 00 00 00 01 ...
```

**C++ Serialization:**
```cpp
BinarySerializer serializer;
serializer.write<uint8_t>(0x11);            // Opcode: ENTITY_POSITIONS
serializer.write<uint16_t>(42);             // Payload size (calculated)
serializer.write<uint32_t>(2);              // Entity count

// Entity 1
serializer.write<uint32_t>(42);             // Network ID
serializer.write<float>(100.5f);            // Position X
serializer.write<float>(200.0f);            // Position Y
serializer.write<float>(5.0f);              // Velocity X
serializer.write<float>(0.0f);              // Velocity Y
serializer.write<uint8_t>(0x01);            // Type: Player

// Entity 2
serializer.write<uint32_t>(100);            // Network ID
serializer.write<float>(400.0f);            // Position X
serializer.write<float>(150.5f);            // Position Y
serializer.write<float>(-3.0f);             // Velocity X
serializer.write<float>(1.0f);              // Velocity Y
serializer.write<uint8_t>(0x02);            // Type: Enemy

server.send_to_all_clients(serializer.data());
```

---

#### Example 4: POWERUP_SPAWNED Packet

**Description:** Server notifies clients that a power-up has spawned.

**Scenario:** Shield power-up (type 0x02) spawned at position (250.0, 180.0) with ID=55

**Byte Layout:**
```
Offset   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14
       ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
       │ 30 │ 0D │ 00 │ 37 │ 00 │ 00 │ 00 │ 00 │ 80 │ 7A │ 43 │ 00 │ 00 │ 34 │ 02 │
       └────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┴────┘
         │    │    │    │    └────┴────┘    │    └────┴────┴────┘    └────┴────┘
         │    │    │    │         │         │         │                    │
         │    │    │    │         │         │         └──── Position Y (float, LE) = 180.0
         │    │    │    │         │         └──────────── Position X (float, LE) = 250.0
         │    │    │    │         └────────────────────── Powerup ID (uint32_t, LE) = 55
         │    └────┴────┘
         │         │
         │         └──────────────────────────────────── Payload Size (uint16_t) = 13
         │
         └────────────────────────────────────────────── Opcode = 0x30 (POWERUP_SPAWNED)
                                                                              │
                                                                              └─ Type (uint8_t) = 0x02 (Shield)

Total Size: 15 bytes
```

**Hexadecimal:** `30 0D 00 37 00 00 00 00 80 7A 43 00 00 34 43 02`

**C++ Serialization:**
```cpp
BinarySerializer serializer;
serializer.write<uint8_t>(0x30);            // Opcode: POWERUP_SPAWNED
serializer.write<uint16_t>(13);             // Payload size
serializer.write<uint32_t>(55);             // Powerup ID
serializer.write<float>(250.0f);            // Position X
serializer.write<float>(180.0f);            // Position Y
serializer.write<uint8_t>(0x02);            // Type: Shield
server.broadcast_to_all_clients(serializer.data());
```

---

### Binary Encoding Notes

#### Little-Endian Format

All multi-byte integers and floats use **little-endian** byte order (least significant byte first):

```
uint16_t value = 300 (0x012C)
Bytes: [2C 01]
       LSB MSB

uint32_t value = 1000 (0x000003E8)
Bytes: [E8 03 00 00]
       LSB       MSB

float value = 10.5
IEEE-754: 0x41280000
Bytes: [00 00 28 41]
       LSB       MSB
```

#### Float Representation (IEEE-754)

All floating-point values use **IEEE-754 single precision** (32-bit):

```
Example: 100.5
Binary: 0 10000101 10010010000000000000000
        │     │              │
        │     │              └── Mantissa (23 bits)
        │     └───────────────── Exponent (8 bits)
        └─────────────────────── Sign (1 bit)

Hexadecimal: 0x42C80000
Little-Endian Bytes: [00 C8 42 42]
```

#### Size Calculations

```
Packet Size = Header Size + Payload Size

Header:
  - Opcode: 1 byte
  - Payload Size: 2 bytes
  = 3 bytes total

Example Payload Sizes:
  - JOIN_LOBBY: 0 bytes
  - PLAYER_INPUT: 1 byte (input mask)
  - ENTITY_POSITIONS (N entities): 4 + N * 21 bytes
    (4 bytes for count + 21 bytes per entity)
  - POWERUP_SPAWNED: 13 bytes (4 ID + 4 X + 4 Y + 1 type)
```

### Opcodes

Defined in `src/Common/Opcodes.hpp`:

```cpp
enum class Opcode : uint8_t {
    // Lobby Phase
    JOIN_LOBBY = 0x01,
    LOBBY_STATUS = 0x02,
    READY_TO_PLAY = 0x03,
    GAME_START = 0x04,
    
    // Game Phase
    PLAYER_INPUT = 0x10,
    ENTITY_POSITIONS = 0x11,
    GAME_INFO = 0x12,
    
    // Level Events
    LEVEL_INFO = 0x20,
    BOSS_SPAWNED = 0x21,
    GAME_OVER = 0x22,
    
    // Powerups
    POWERUP_SPAWNED = 0x30,
    POWERUP_CHOICE = 0x31,
    POWERUP_STATUS = 0x32,
    
    // Weapons
    WEAPON_UPGRADE_AVAILABLE = 0x40,
    WEAPON_UPGRADE_CHOICE = 0x41,
    WEAPON_UPGRADE_CONFIRM = 0x42,
    
    // Utility
    PING = 0xFE,
    ERROR = 0xFF
};
```

### Message Examples

#### JOIN_LOBBY (Client → Server)

```cpp
struct JoinLobbyPacket {
    uint8_t opcode = static_cast<uint8_t>(Opcode::JOIN_LOBBY);
    uint16_t payload_size = 0;  // No payload
};

// Send
std::vector<uint8_t> packet(3);
packet[0] = static_cast<uint8_t>(Opcode::JOIN_LOBBY);
packet[1] = 0;  // payload_size low byte
packet[2] = 0;  // payload_size high byte
network_client.send(packet);
```

#### PLAYER_INPUT (Client → Server)

```cpp
struct PlayerInputPacket {
    uint8_t opcode = static_cast<uint8_t>(Opcode::PLAYER_INPUT);
    uint16_t payload_size = 1;
    uint8_t input_mask;  // Bit flags for keys
};

// Input mask bits
constexpr uint8_t INPUT_UP    = 0x01;
constexpr uint8_t INPUT_DOWN  = 0x02;
constexpr uint8_t INPUT_LEFT  = 0x04;
constexpr uint8_t INPUT_RIGHT = 0x08;
constexpr uint8_t INPUT_SHOOT = 0x10;

// Create and send
uint8_t input_mask = INPUT_UP | INPUT_RIGHT | INPUT_SHOOT;
std::vector<uint8_t> packet = {
    static_cast<uint8_t>(Opcode::PLAYER_INPUT),
    1, 0,  // payload_size = 1
    input_mask
};
network_client.send(packet);
```

#### ENTITY_POSITIONS (Server → Clients)

```cpp
struct EntityPositionPacket {
    uint8_t opcode = static_cast<uint8_t>(Opcode::ENTITY_POSITIONS);
    uint16_t payload_size;
    uint32_t entity_count;
    // For each entity:
    struct EntityData {
        uint32_t network_id;
        float x, y;
        float vx, vy;
        uint8_t entity_type;
    } entities[entity_count];
};

// Serialize
BinarySerializer serializer;
serializer.write<uint8_t>(Opcode::ENTITY_POSITIONS);
serializer.write<uint16_t>(payload_size);
serializer.write<uint32_t>(entity_count);
for (const auto& [id, pos, vel, type] : entities) {
    serializer.write<uint32_t>(id);
    serializer.write<float>(pos.x);
    serializer.write<float>(pos.y);
    serializer.write<float>(vel.vx);
    serializer.write<float>(vel.vy);
    serializer.write<uint8_t>(type);
}
server.send_to_client(client_id, serializer.data());
```

##  Communication Flow

### Lobby Phase

```
Client                          Server
  │                               │
  ├──► JOIN_LOBBY ───────────────>│
  │                               ├─> Add client to session
  │                               ├─> Assign network ID
  │<────── LOBBY_STATUS ──────────┤
  │        (players: 1/4)         │
  │                               │
  ├──► READY_TO_PLAY ────────────>│
  │                               ├─> Mark player ready
  │<────── LOBBY_STATUS ──────────┤
  │        (ready: 1/4)           │
  │                               │
  │    [All players ready]        │
  │                               │
  │<────── GAME_START ─────────────┤
  │        (your_id: 1)           │
  │                               │
```

### Game Phase (60Hz Updates)

```
Client (60 FPS)                 Server (60 Hz)
  │                               │
  ├──► PLAYER_INPUT ─────────────>│
  │    (every frame)              ├─> InputHandler
  │                               ├─> Update game logic
  │                               ├─> Collision detection
  │<────── ENTITY_POSITIONS ──────┤
  │    (all entities)             │
  │<────── GAME_INFO ─────────────┤
  │    (score, health, etc.)      │
  │                               │
  │    [Enemy killed]             │
  │                               │
  │<────── POWERUP_SPAWNED ────────┤
  ├──► POWERUP_CHOICE ───────────>│
  │                               ├─> PowerupHandler
  │<────── POWERUP_STATUS ─────────┤
  │                               │
  │    [Level complete]           │
  │                               │
  │<────── LEVEL_INFO ─────────────┤
  │    (next level)               │
  │                               │
```

##  Broadcaster Pattern

The server uses specialized **Broadcaster** classes to serialize and send state:

### EntityBroadcaster

```cpp
// server/include/network/EntityBroadcaster.hpp
class EntityBroadcaster {
public:
    void broadcast_entity_positions(
        UDPServer& server,
        registry& reg,
        const std::unordered_map<int, entity>& client_entity_ids
    );
};
```

**What it does:**
1. Iterates over all entities with `position`, `velocity`, `network_id`
2. Serializes data into binary packet
3. Sends unicast to each connected client

### LobbyBroadcaster

```cpp
// server/include/network/LobbyBroadcaster.hpp
class LobbyBroadcaster {
public:
    void broadcast_lobby_status(
        UDPServer& server,
        const std::unordered_map<int, bool>& client_ready_status
    );
};
```

### GameBroadcaster

```cpp
// server/include/network/GameBroadcaster.hpp
class GameBroadcaster {
public:
    void broadcast_level_info(UDPServer& server, int level, float time_elapsed);
    void broadcast_game_over(UDPServer& server);
};
```

### PowerupBroadcaster

```cpp
// server/include/network/PowerupBroadcaster.hpp
class PowerupBroadcaster {
public:
    void broadcast_powerup_spawned(UDPServer& server, uint32_t powerup_id, 
                                    float x, float y, uint8_t type);
    void broadcast_powerup_status(UDPServer& server, registry& reg, 
                                   const std::unordered_map<int, entity>& clients);
};
```

**Pattern Benefits:**
-  Separates networking from game logic
-  Reusable across different game modes
-  Easy to test independently
-  Clear responsibility (single purpose)

##  Reliability & Error Handling

### UDP Challenges

UDP is **unreliable** by design:
-  Packets can be lost
-  Packets can arrive out of order
-  No connection state

**Our Solutions:**

### 1. Redundancy

```cpp
// Send critical state every frame
void GameSession::update() {
    // ... game logic ...
    
    // Broadcast every tick (60Hz)
    _entity_broadcaster.broadcast_entity_positions(server, _registry, _client_entity_ids);
    _game_broadcaster.broadcast_game_info(server, score, health, level);
}
```

Even if 1-2 packets drop, next frame arrives quickly.

### 2. Client-Side Prediction

```cpp
// Client predicts movement locally
void Game::update(float dt) {
    // Apply input immediately (prediction)
    update_local_player_position(dt);
    
    // Correct when server update arrives
    if (server_update_received) {
        reconcile_with_server_state();
    }
}
```

### 3. Interpolation

```cpp
// Smooth entity movement between updates
void Game::render() {
    for (auto& entity : entities_) {
        // Interpolate between last two server positions
        float t = time_since_last_update / update_interval;
        entity.render_position = lerp(entity.prev_pos, entity.target_pos, t);
    }
}
```

### 4. Heartbeat (Ping)

```cpp
// Detect disconnections
void NetworkClient::send_ping() {
    std::vector<uint8_t> packet = {
        static_cast<uint8_t>(Opcode::PING),
        0, 0  // No payload
    };
    send(packet);
}

// Server: Disconnect if no ping for 5 seconds
if (time_since_last_packet > 5.0f) {
    disconnect_client(client_id);
}
```

##  Performance Optimization

### 1. Minimize Packet Size

```cpp
//  Bad: Send full entity data every frame (50 bytes/entity)
struct EntityFull {
    uint32_t id;
    float x, y, vx, vy;
    uint8_t type;
    uint8_t health;
    uint32_t texture_id;
    // ... many more fields
};

//  Good: Send only position delta (13 bytes/entity)
struct EntityPosition {
    uint32_t id;        // 4 bytes
    float x, y;         // 8 bytes
    uint8_t type;       // 1 byte
};  // Total: 13 bytes
```

**For 100 entities:**
- Bad: 5,000 bytes/frame × 60 FPS = 300 KB/s
- Good: 1,300 bytes/frame × 60 FPS = 78 KB/s

### 2. Batch Updates

```cpp
//  Send all entities in one packet
void broadcast_entity_positions(...) {
    BinarySerializer serializer;
    serializer.write<uint8_t>(Opcode::ENTITY_POSITIONS);
    serializer.write<uint32_t>(entity_count);
    
    for (const auto& entity : entities) {
        serializer.write<uint32_t>(entity.id);
        serializer.write<float>(entity.pos.x);
        serializer.write<float>(entity.pos.y);
    }
    
    server.send_to_all_clients(serializer.data());  // One packet
}
```

### 3. Prioritization

```cpp
// Update critical entities more frequently
if (entity.is_player() || entity.is_boss()) {
    send_every_frame();
} else if (entity.is_enemy()) {
    send_every_2_frames();
} else {  // Background objects
    send_every_5_frames();
}
```

##  Testing Network Code

### Unit Tests

```cpp
// tests/network/test_udp_server.cpp
TEST(UDPServerTest, ClientRegistration) {
    asio::io_context io;
    server::UDPServer server(io, "127.0.0.1", 12345);
    
    // Simulate client connection
    NetworkPacket packet;
    packet.sender = /* client endpoint */;
    packet.data = create_join_packet();
    
    server.process_packet(packet);
    
    EXPECT_EQ(server.get_client_count(), 1);
}

TEST(NetworkClientTest, SendReceive) {
    asio::io_context io;
    NetworkClient client(io);
    
    client.connect("127.0.0.1", "12345");
    client.send({0x01, 0x00, 0x00});  // JOIN_LOBBY
    
    std::vector<uint8_t> response;
    ASSERT_TRUE(client.try_pop_message(response));
    EXPECT_EQ(response[0], static_cast<uint8_t>(Opcode::LOBBY_STATUS));
}
```

### Integration Tests

```cpp
// tests/integration/test_multiplayer.cpp
TEST(MultiplayerTest, TwoPlayersJoinAndPlay) {
    // Start server
    ServerFixture server;
    server.start();
    
    // Connect two clients
    ClientFixture client1, client2;
    client1.connect("127.0.0.1", "12345");
    client2.connect("127.0.0.1", "12345");
    
    // Both ready
    client1.send_ready();
    client2.send_ready();
    
    // Game should start
    EXPECT_EQ(client1.get_state(), GameState::InGame);
    EXPECT_EQ(client2.get_state(), GameState::InGame);
}
```

##  Security Considerations

### 1. Input Validation

```cpp
// Server validates all client input
void InputHandler::handle_player_input(const NetworkPacket& packet) {
    if (packet.data.size() < 4) {
        // Invalid packet size
        return;
    }
    
    uint8_t input_mask = packet.data[3];
    
    // Sanity check: Only valid input bits
    constexpr uint8_t VALID_INPUTS = 0x1F;  // 5 bits
    if ((input_mask & ~VALID_INPUTS) != 0) {
        // Invalid input bits
        return;
    }
    
    // Process valid input
    // ...
}
```

### 2. Rate Limiting

```cpp
// Prevent input flooding
class RateLimiter {
    std::unordered_map<int, std::queue<float>> client_timestamps_;
    
public:
    bool allow(int client_id, float current_time) {
        auto& timestamps = client_timestamps_[client_id];
        
        // Remove old timestamps (older than 1 second)
        while (!timestamps.empty() && current_time - timestamps.front() > 1.0f) {
            timestamps.pop();
        }
        
        // Allow max 100 inputs per second
        if (timestamps.size() >= 100) {
            return false;  // Rate limited
        }
        
        timestamps.push(current_time);
        return true;
    }
};
```

### 3. Server Authority

```cpp
//  Server validates all actions
void handle_shoot_request(int client_id) {
    auto& player = get_player(client_id);
    
    // Check cooldown (prevent rapid fire hacks)
    if (current_time - player.last_shot_time < 0.2f) {
        return;  // Too soon, ignore
    }
    
    // Check ammo (if applicable)
    if (player.ammo <= 0) {
        return;  // No ammo, ignore
    }
    
    // Authorized: Spawn projectile
    spawn_projectile(player);
    player.last_shot_time = current_time;
    player.ammo--;
}
```

##  Related Documentation

- [Architecture Overview](overview.md) - High-level project structure
- [ECS System](ecs.md) - Entity Component System details
- [Server Refactoring](../REFACTOR_SERVER.md) - Server architecture details
- [Client Refactoring](../REFACTOR_CLIENT.md) - Client architecture details

##  Further Reading

- [Gaffer On Games - Networking](https://gafferongames.com/categories/networking/) - Authoritative series
- [Valve Source Engine Networking](https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking) - Industry best practices
- [Fast-Paced Multiplayer](https://www.gabrielgambetta.com/client-server-game-architecture.html) - Client prediction & reconciliation
- [ASIO Documentation](https://think-async.com/Asio/) - Async I/O library
