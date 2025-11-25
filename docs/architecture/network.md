# R-Type Network Protocol Specification (RT-NPS)

**Version:** 1.1 (Stable)
**Date:** November 2025
**Status:** Final Draft
**Architecture:** Client-Server / Authoritative Server
**Format:** Binary (Little-Endian)

---

## 1. Fundamentals & Standards

To ensure low latency and bandwidth efficiency, the R-Type protocol uses raw binary data. No text-based formats (JSON/XML) are allowed during the gameplay phase.

### 1.1 Data Representation
* **Endianness:** **Little-Endian** (Standard Intel/AMD architecture).
* **Alignment:** 1 Byte packing (`#pragma pack(push, 1)` in C++).
* **Strings:** Fixed-size `char` arrays, padded with `\0`.
* **Types:**
    * `uint8_t` (1 byte)
    * `uint16_t` (2 bytes)
    * `uint32_t` (4 bytes)
    * `float` (4 bytes, IEEE 754)

### 1.2 Protocol Flow
The communication is divided into two distinct phases:
1.  **Lobby Phase (TCP):** Used for reliable tasks (Login, Room selection, Chat, Waiting for players).
2.  **Game Phase (UDP):** Used for real-time tasks (Movement, Shooting, Health updates).

---

## 2. Packet Structure (The Envelope)

**Every single packet** (TCP & UDP) sent between Client and Server must start with this header.

### 2.1 The Header (6 Bytes)

```cpp
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t magicNumber;  // Safety check: Always 0xB542
    uint16_t sequenceId;   // Order check: Incremental ID (UDP Reliability)
    uint8_t  commandId;    // OpCode: The type of message
    uint8_t  payloadSize;  // Size of the data following this header
};
#pragma pack(pop)
```

* **MagicNumber:** If a received packet does not start with `0xB542`, it must be discarded immediately (security/noise filtering).
* **SequenceId:** Used in UDP to detect out-of-order packets.
    * *Client Logic:* If `received.seq < last_received.seq`, discard the packet (it's an old "ghost" packet).

-----

## 3. Command Table (OpCodes)

This table defines the values for `commandId`.

### TCP Commands (Lobby & Connection)

| Hex | Mnemonic | Direction | Description |
| :--- | :--- | :--- | :--- |
| **0x01** | `REQ_CONNECT` | C -> S | Initial connection request. |
| **0x02** | `RES_CONNECT` | S -> C | Connection accepted/rejected. |
| **0x03** | `REQ_CREATE_ROOM` | C -> S | Request to create a new game room. |
| **0x04** | `REQ_JOIN_ROOM` | C -> S | Request to join a specific room ID. |
| **0x05** | `NTF_ROOM_CREATED` | S -> C | Confirm room creation + ID. |
| **0x06** | `NTF_ROOM_JOINED` | S -> C | Confirm room join + current players. |
| **0x07** | `REQ_START_GAME` | C -> S | The host starts the game. |
| **0x08** | `NTF_GAME_START` | S -> C | Game starting. Switch to UDP. |

### UDP Commands (In-Game)

| Hex | Mnemonic | Direction | Description |
| :--- | :--- | :--- | :--- |
| **0x10** | `REQ_INPUT` | C -> S | Player input state (pressed keys). |
| **0x11** | `NTF_ENTITY_SPAWN` | S -> C | Create a new visual entity. |
| **0x12** | `NTF_ENTITY_DESTROY` | S -> C | Delete an entity. |
| **0x13** | `NTF_ENTITY_POS` | S -> C | Update position/velocity (Snapshot). |
| **0x14** | `NTF_PLAYER_STAT` | S -> C | Update HUD (Health, Score, Charge). |
| **0x15** | `REQ_PING` | Any | Keep-alive heartbeat. |

-----

## 4. Payload Specifications

Data structures sent after the Header.

### 4.1 Connection & Lobby (TCP)

**`0x01` - REQ_CONNECT**

```cpp
struct LoginRequest {
    char username[32]; // Null-terminated if < 32 chars
};
```

**`0x02` - RES_CONNECT**

```cpp
struct LoginResponse {
    uint8_t  status;    // 1 = Success, 0 = Fail
    uint32_t playerId;  // The unique Client ID assigned by Server
};
```

**`0x08` - NTF_GAME_START**
*Critical:* Tells the client the TCP phase is over.

```cpp
struct GameStartPacket {
    uint16_t udpPort; // The UDP port to send inputs to
    uint32_t playerEntityId; // The ECS Entity ID of the player's ship
};
```

-----

### 4.2 Game Phase (UDP)

**`0x10` - REQ_INPUT (Client -> Server)**
Sent every tick (e.g., 60 times/sec).

```cpp
struct InputPacket {
    uint8_t inputMask; // for efficiency
};
```

* **Bitmask Layout:**
    * Bit 0: UP
    * Bit 1: DOWN
    * Bit 2: LEFT
    * Bit 3: RIGHT
    * Bit 4: SHOOT (Primary Fire)
    * Bit 5: BOMB (Secondary)

> **Note on Charged Shot:** The Client does NOT send "Charged Shot". The client sends "SHOOT" (Bit 4) continuously. The **Server** tracks how long the bit is held. If held > 1 second, the Server spawns a Charged Bullet when the key is released.

**`0x11` - NTF_ENTITY_SPAWN (Server -> Client)**

```cpp
struct EntitySpawnPacket {
    uint32_t entityId;
    uint8_t  type;    // See Section 5: Entity Types
    float    x;       // Starting X
    float    y;       // Starting Y
    float    velX;    // Initial Velocity X
    float    velY;    // Initial Velocity Y
    float    rotation;// Orientation in degrees
};
```

**`0x12` - NTF_ENTITY_DESTROY (Server -> Client)**

```cpp
struct EntityDestroyPacket {
    uint32_t entityId;
};
```

**`0x13` - NTF_ENTITY_POS (Server -> Client)**
Sent frequently for moving objects.

```cpp
struct EntityPositionPacket {
    uint32_t entityId;
    float    x;
    float    y;
    float    velX;
    float    velY;
};
```

-----

## 5. Entity Type Registry

Mapping between `uint8_t type` and game assets.

| Type ID | Code Name | Description / Asset |
| :--- | :--- | :--- |
| **1** | `PLAYER_BLUE` | Player 1 Ship |
| **2** | `PLAYER_RED` | Player 2 Ship |
| **3** | `PLAYER_GREEN` | Player 3 Ship |
| **4** | `PLAYER_YELLOW` | Player 4 Ship |
| **10** | `MOB_BASIC` | Weak enemy (PataPata) |
| **11** | `MOB_ARMORED` | Round armored enemy |
| **12** | `BOSS_STAGE1` | Huge boss sprite |
| **20** | `BULLET_PLAYER` | Small plasma shot |
| **21** | `BULLET_CHARGE` | Large charged shot |
| **22** | `BULLET_ENEMY` | Enemy red orb |

-----

## 6. Reliability & Edge Cases (CRITICAL)

Since UDP is unreliable, the engine must implement the following safeguards:

### 6.1 Redundancy for Critical Events

Packets `NTF_ENTITY_SPAWN` (0x11) and `NTF_ENTITY_DESTROY` (0x12) are **critical**. If lost, the game breaks (invisible monsters).

* **Rule:** The Server must send these packets **3 times consecutively** (e.g., Frame N, N+1, N+2).
* **Client logic:** If the client receives a Spawn request for an `entityId` that already exists in the local Registry, it must **ignore** the duplicate.

### 6.2 Out-of-Order Handling

Using the `sequenceId` in the Header:

1.  Client stores `last_seq_id`.
2.  Incoming Packet P arrives.
3.  `if (P.sequenceId < last_seq_id)` -> **DROP P** (It's outdated).
4.  `if (P.sequenceId > last_seq_id)` -> **PROCESS P** and update `last_seq_id`.

### 6.3 Timeout / Disconnection

* The Server maintains a `last_packet_time` for each client.
* If no packet is received for **5 seconds**, the Client is considered timed out.
* **Action:** Server deletes the Player Entity and notifies others via `NTF_ENTITY_DESTROY`.
