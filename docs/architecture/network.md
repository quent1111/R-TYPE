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
    
    // Client management
    int register_client(const asio::ip::udp::endpoint& endpoint);
    std::vector<int> remove_inactive_clients(std::chrono::seconds timeout);
    size_t get_client_count();
    std::map<int, asio::ip::udp::endpoint> get_all_clients();
    void disconnect_client(int client_id);
    
    // Send operations
    void send_to_all(const std::vector<uint8_t>& data);
    void send_to_clients(const std::vector<int>& client_ids, const std::vector<uint8_t>& data);
    void send_to_client(int client_id, const std::vector<uint8_t>& data);
    void send_to_endpoint(const asio::ip::udp::endpoint& endpoint,
                          const std::vector<uint8_t>& data);
    
    // Reliable delivery
    void send_reliable(int client_id, uint8_t opcode, const std::vector<uint8_t>& payload);
    void send_ack(int client_id, uint32_t sequence_id);
    void handle_ack(int client_id, uint32_t sequence_id);
    
    // Queue operations
    bool get_input_packet(NetworkPacket& packet);
    void queue_output_packet(NetworkPacket packet);
    size_t get_input_queue_size() const;
    
    // Lifecycle
    void run_network_loop();
    void stop();
    
private:
    asio::io_context& io_context_;
    std::unique_ptr<asio::ip::udp::socket> socket_;
    asio::ip::udp::endpoint remote_endpoint_;
    std::map<int, ClientEndpoint> clients_;
    std::mutex clients_mutex_;
    ThreadSafeQueue<NetworkPacket> input_queue_;
    std::map<int, RType::ClientReliabilityState> client_reliability_;
    std::thread retry_thread_;
    
    void start_receive();
    void handle_receive(std::error_code ec, std::size_t bytes_received);
    void retry_unacked_packets();
};

} // namespace server
```

**Key Features:**
- **Async I/O**: Boost.Asio for non-blocking UDP operations
- **Client Tracking**: Maps client ID → UDP endpoint with timeout detection
- **Thread-Safe Queue**: `ThreadSafeQueue<NetworkPacket>` for game/network thread communication
- **Reliable Delivery**: Optional ACK-based reliability layer for critical messages
- **Retry Mechanism**: Background thread retries unacknowledged packets

**Usage Example:**

```cpp
// In server main thread
asio::io_context io_context;
server::UDPServer server(io_context, "0.0.0.0", 12345);

// Network thread runs async I/O
std::thread network_thread([&server]() {
    server.run_network_loop();  // Runs until stop() is called
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
    std::vector<uint8_t> entity_data = serialize_entities();
    server.send_to_all(entity_data);
    
    // 4. Fixed timestep
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
    NetworkClient(const std::string& host, unsigned short port,
                  ThreadSafeQueue<GameToNetwork::Message>& game_to_net,
                  ThreadSafeQueue<NetworkToGame::Message>& net_to_game);
    
    // Message handlers (called from network thread)
    void decode_entities(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_login_ack(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_lobby_status(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_start_game(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_level_start(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_level_progress(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_powerup_selection(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_boss_spawn(const std::vector<uint8_t>& buffer, std::size_t received);
    void decode_game_over(const std::vector<uint8_t>& buffer, std::size_t received);
    
    // Send operations (called from game thread via queues)
    void send_login();
    void send_input(uint8_t input_mask);
    void send_ready(bool ready);
    void send_powerup_choice(uint8_t choice);
    void send_powerup_activate(uint8_t powerup_type);
    
    // Lifecycle
    void run();
    void stop();
    
    uint32_t get_my_network_id() const;
    
private:
    asio::io_context io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    std::array<uint8_t, 65536> recv_buffer_;
    std::thread network_thread_;
    
    // Thread-safe queues for communication
    ThreadSafeQueue<GameToNetwork::Message>& game_to_network_queue_;
    ThreadSafeQueue<NetworkToGame::Message>& network_to_game_queue_;
    
    uint32_t my_network_id_ = 0;
    std::atomic<bool> running_;
    
    void start_receive();
    void handle_receive(std::error_code ec, std::size_t bytes_received);
    void receive_loop();
    void send_loop();
};
```

**Usage Example:**

```cpp
// In client main
ThreadSafeQueue<GameToNetwork::Message> game_to_net;
ThreadSafeQueue<NetworkToGame::Message> net_to_game;

NetworkClient network_client("127.0.0.1", 12345, game_to_net, net_to_game);

// Start network thread
std::thread network_thread([&network_client]() {
    network_client.run();  // Handles send/receive loops
});

// Game loop
while (running) {
    // 1. Send input to network thread
    GameToNetwork::InputMessage input_msg;
    input_msg.input_mask = calculate_input_mask();
    game_to_net.push(input_msg);
    
    // 2. Receive server updates from network thread
    NetworkToGame::Message message;
    while (net_to_game.try_pop(message)) {
        if (message.type == NetworkToGame::MessageType::EntityUpdate) {
            update_entities(message.entities);
        }
        // Handle other message types...
    }
    
    // 3. Update local state
    update_client(dt);
    
    // 4. Render
    render();
}

network_client.stop();
network_thread.join();
```

**Key Features:**
- **Queue-Based Communication**: Game thread and network thread never share direct state
- **Typed Messages**: `GameToNetwork::Message` and `NetworkToGame::Message` structs
- **Magic Number Validation**: All packets start with 0xB542 (MAGIC_NUMBER)

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

### ThreadSafeQueue

Located in `client/include/common/SafeQueue.hpp` and `server/include/common/SafeQueue.hpp`

```cpp
template <typename T>
class ThreadSafeQueue {
public:
    void push(const T& item);
    void push(T&& item);
    bool try_pop(T& item);
    bool empty() const;
    size_t size() const;
    
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};
```

**Thread-safe operations:**
- `push()`: Adds item and notifies waiting threads
- `try_pop()`: Non-blocking pop, returns false if empty
- Used for game-to-network and network-to-game communication

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
enum class OpCode : uint8_t {
    // Authentication
    Login = 0x01,
    LoginAck = 0x02,
    Keepalive = 0x03,
    
    // Input
    Input = 0x10,
    
    // Entities
    EntitySpawn = 0x11,
    EntityDestroy = 0x12,
    EntityPosition = 0x13,
    
    // Lobby Phase
    PlayerReady = 0x20,
    LobbyStatus = 0x21,
    StartGame = 0x22,
    ListLobbies = 0x23,
    CreateLobby = 0x24,
    JoinLobby = 0x25,
    LeaveLobby = 0x26,
    LobbyJoined = 0x27,
    LobbyLeft = 0x28,
    SelectLevel = 0x29,
    ListLevels = 0x2A,
    LevelList = 0x2B,
    
    // Level Events
    LevelStart = 0x30,
    LevelComplete = 0x31,
    WeaponUpgradeChoice = 0x32,
    LevelProgress = 0x33,
    
    // Powerups
    PowerUpChoice = 0x34,
    PowerUpActivate = 0x35,
    PowerUpStatus = 0x36,
    PowerUpCards = 0x37,
    ActivableSlots = 0x38,
    RequestGameState = 0x39,
    
    // Game Over
    GameOver = 0x40,
    
    // Boss
    BossSpawn = 0x50,
    
    // Admin
    AdminLogin = 0xA0,
    AdminLoginAck = 0xA1,
    AdminCommand = 0xA2,
    AdminResponse = 0xA3,
    AdminLogout = 0xA4,
    
    // Magic bytes
    MagicByte1 = 0x42,
    MagicByte2 = 0xB5
};

enum class EntityType : uint8_t {
    Player       = 0x01,
    Enemy        = 0x02,
    Projectile   = 0x03,
    Powerup      = 0x04,
    Obstacle     = 0x05,
    Enemy2       = 0x06,
    Enemy3       = 0x07,
    Boss         = 0x08,
    HomingEnemy  = 0x09,
    Ally         = 0x0A,
    LaserBeam    = 0x0B,
    SupportDrone = 0x0C,
    MissileDrone = 0x0D,
    Enemy4       = 0x0E,
    Enemy5       = 0x0F,
    SerpentNest  = 0x10,
    SerpentHead  = 0x11,
    SerpentBody  = 0x12,
    SerpentScale = 0x13,
    SerpentTail  = 0x14,
    SerpentHoming = 0x15,
    SerpentLaser  = 0x16,
    SerpentLaserSegment = 0x17,
    SerpentScream = 0x18,
    SerpentLaserCharge = 0x19,
    FlyingEnemy = 0x1A,
    CompilerBoss = 0x1B,
    CompilerPart1 = 0x1C,
    CompilerPart2 = 0x1D,
    CompilerPart3 = 0x1E,
    CompilerExplosion = 0x1F,
    CustomEnemy = 0x30,
    CustomBoss = 0x31,
    CustomProjectile = 0x32
};

struct MagicNumber {
    static constexpr uint16_t VALUE = 0xB542;
    static constexpr uint8_t BYTE1 = static_cast<uint8_t>(OpCode::MagicByte1);
    static constexpr uint8_t BYTE2 = static_cast<uint8_t>(OpCode::MagicByte2);
};
```

### Message Examples

#### Login (Client → Server)

```cpp
// Send login request
std::vector<uint8_t> packet(3);
packet[0] = static_cast<uint8_t>(OpCode::Login);
packet[1] = 0;  // payload_size low byte
packet[2] = 0;  // payload_size high byte
// Sent via game_to_network queue
```

#### Input (Client → Server)

```cpp
// Input mask bits
constexpr uint8_t INPUT_UP    = 0x01;
constexpr uint8_t INPUT_DOWN  = 0x02;
constexpr uint8_t INPUT_LEFT  = 0x04;
constexpr uint8_t INPUT_RIGHT = 0x08;
constexpr uint8_t INPUT_SHOOT = 0x10;

// Create and send input
uint8_t input_mask = INPUT_UP | INPUT_RIGHT | INPUT_SHOOT;
network_client.send_input(input_mask);
```

#### EntityPosition (Server → Clients)

```cpp
// Serialize entity positions
RType::CompressionSerializer serializer;
serializer.write<uint8_t>(static_cast<uint8_t>(OpCode::EntityPosition));
serializer.write<uint16_t>(payload_size);
serializer.write<uint32_t>(entity_count);

for (const auto& [id, entity] : entities) {
    serializer.write<uint32_t>(id);
    serializer.write<float>(entity.position.x);
    serializer.write<float>(entity.position.y);
    serializer.write<float>(entity.velocity.vx);
    serializer.write<float>(entity.velocity.vy);
    serializer.write<uint8_t>(static_cast<uint8_t>(entity.type));
}

server.send_to_all(serializer.data());
```

### Communication Flow

### Lobby Phase

```
Client                          Server
  │                               │
  ├──► Login ────────────────────>│
  │                               ├─> Register client, assign network ID
  │<────── LoginAck ──────────────┤
  │        (your_id: 1)           │
  │                               │
  ├──► JoinLobby ────────────────>│
  │                               ├─> Add to lobby
  │<────── LobbyStatus ───────────┤
  │        (players: 1/4)         │
  │                               │
  ├──► PlayerReady ──────────────>│
  │                               ├─> Mark player ready
  │<────── LobbyStatus ───────────┤
  │        (ready: 1/4)           │
  │                               │
  │    [All players ready]        │
  │                               │
  │<────── StartGame ──────────────┤
  │                               │
```

### Game Phase (60Hz Updates)

```
Client (60 FPS)                 Server (60 Hz)
  │                               │
  ├──► Input ────────────────────>│
  │    (every frame)              ├─> InputHandler
  │                               ├─> Update game logic
  │                               ├─> Collision detection
  │<────── EntityPosition ────────┤
  │    (all entities)             │
  │<────── LevelProgress ─────────┤
  │    (score, enemies left)      │
  │                               │
  │    [Enemy killed]             │
  │                               │
  │<────── PowerUpCards ───────────┤
  ├──► PowerUpChoice ────────────>│
  │                               ├─> PowerupHandler
  │<────── PowerUpStatus ──────────┤
  │                               │
  │    [Level complete]           │
  │                               │
  │<────── LevelComplete ──────────┤
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
        const std::unordered_map<int, std::size_t>& client_entity_ids,
        const std::vector<int>& lobby_client_ids
    );
    
    void send_full_game_state_to_client(
        UDPServer& server,
        registry& reg,
        const std::unordered_map<int, std::size_t>& client_entity_ids,
        int client_id
    );
};
```

**What it does:**
1. Iterates over all entities with `position`, `velocity`, `network_id`
2. Serializes data into compressed binary packet using `CompressionSerializer`
3. Sends to all clients in lobby or specific client for full state sync

### LobbyBroadcaster

```cpp
// server/include/network/LobbyBroadcaster.hpp
class LobbyBroadcaster {
public:
    void broadcast_lobby_status(
        UDPServer& server,
        const std::unordered_map<int, bool>& client_ready_status,
        const std::vector<int>& lobby_client_ids
    );
};
```

### GameBroadcaster

```cpp
// server/include/network/GameBroadcaster.hpp
class GameBroadcaster {
public:
    void broadcast_level_info(UDPServer& server, registry& reg,
                              const std::vector<int>& lobby_client_ids);
    void broadcast_level_complete(UDPServer& server, registry& reg,
                                  const std::vector<int>& lobby_client_ids);
    void broadcast_level_start(UDPServer& server, uint8_t level,
                               const std::string& custom_level_id,
                               const std::vector<int>& lobby_client_ids);
    void broadcast_boss_spawn(UDPServer& server, const std::vector<int>& lobby_client_ids);
    void broadcast_start_game(UDPServer& server, const std::vector<int>& lobby_client_ids);
    void broadcast_game_over(UDPServer& server, const std::vector<int>& lobby_client_ids);
};
```

### PowerupBroadcaster

```cpp
// server/include/network/PowerupBroadcaster.hpp
class PowerupBroadcaster {
public:
    void broadcast_powerup_selection(UDPServer& server,
                                    const std::vector<int>& lobby_client_ids);
    void broadcast_powerup_cards(UDPServer& server, int client_id,
                                const std::vector<powerup::PowerupCard>& cards);
    void broadcast_powerup_status(UDPServer& server, registry& reg,
                                 const std::unordered_map<int, std::size_t>& client_entity_ids,
                                 const std::vector<int>& lobby_client_ids);
    void broadcast_activable_slots(UDPServer& server, int client_id,
                                  const powerup::PlayerPowerups::ActivableSlot slots[2]);
};
```

**Pattern Benefits:**
- Separates networking from game logic
- Reusable across different game modes
- Easy to test independently
- Uses `CompressionSerializer` for bandwidth optimization
- Clear responsibility (single purpose)

##  Reliability & Error Handling

### UDP Challenges

UDP is **unreliable** by design:
- Packets can be lost
- Packets can arrive out of order
- No connection state

**Our Solutions:**

### 1. Redundancy

```cpp
// Send critical state every frame
void GameSession::update() {
    // ... game logic ...
    
    // Broadcast every tick (60Hz)
    _entity_broadcaster.broadcast_entity_positions(server, _registry, 
                                                  _client_entity_ids, _client_ids);
}
```

Even if 1-2 packets drop, next frame arrives quickly (16ms later).

### 2. Reliable Delivery Layer

For critical messages (level start, game over), the server uses ACK-based reliability:

```cpp
// Send with reliability
server.send_reliable(client_id, OpCode::LevelStart, payload);

// Client sends ACK
server.handle_ack(client_id, sequence_id);

// Background thread retries unacked packets
server.retry_unacked_packets();
```

**Retry mechanism:**
- Packets stored in `ClientReliabilityState` until ACK received
- Retry thread resends after timeout (100ms, 200ms, 400ms exponential backoff)
- Max retries before considering client disconnected

### 3. Client-Side Interpolation

```cpp
// Client smooths entity movement between updates
void Game::render() {
    for (auto& entity : entities_) {
        // Interpolate between last two server positions
        float t = time_since_last_update / update_interval;
        entity.render_position = lerp(entity.prev_pos, entity.target_pos, t);
    }
}
```

### 4. Keepalive / Timeout Detection

```cpp
// Server: Disconnect if no packets for 10 seconds
std::vector<int> inactive = server.remove_inactive_clients(std::chrono::seconds(10));

// Client: Send keepalive if no input sent recently
if (time_since_last_send > 2.0f) {
    network_client.send_login();  // Acts as keepalive
}
```

##  Performance Optimization

### 1. Minimize Packet Size

```cpp
// Good: Send only essential position data (21 bytes/entity)
struct EntityPosition {
    uint32_t id;        // 4 bytes
    float x, y;         // 8 bytes
    float vx, vy;       // 8 bytes
    uint8_t type;       // 1 byte
};  // Total: 21 bytes

// With compression (CompressionSerializer):
// - Delta encoding for positions
// - Quantization for velocities
// - Reduces to ~15 bytes/entity average
```

**For 100 entities:**
- Uncompressed: 2,100 bytes/frame × 60 FPS = 126 KB/s
- Compressed: ~1,500 bytes/frame × 60 FPS = 90 KB/s

### 2. Batch Updates

```cpp
// Send all entities in one packet (not one packet per entity)
void broadcast_entity_positions(...) {
    RType::CompressionSerializer serializer;
    serializer.write<uint8_t>(OpCode::EntityPosition);
    serializer.write<uint32_t>(entity_count);
    
    for (const auto& entity : entities) {
        serializer.write<uint32_t>(entity.id);
        serializer.write<float>(entity.pos.x);
        serializer.write<float>(entity.pos.y);
        serializer.write<float>(entity.vel.x);
        serializer.write<float>(entity.vel.y);
        serializer.write<uint8_t>(entity.type);
    }
    
    server.send_to_all(serializer.data());  // One packet for all entities
}
```

### 3. Selective Updates

```cpp
// Only send entities visible to player
// (Implementation in EntityBroadcaster filters by screen bounds)
for (const auto& entity : entities) {
    if (is_visible_to_client(entity, client_viewport)) {
        serialize_entity(entity);
    }
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
