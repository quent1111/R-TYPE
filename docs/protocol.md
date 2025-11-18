# R-Type Network Protocol Specification (RFC)

Version: 1.0  
Status: Draft  
Authors: R-Type Team  

---

## 1. Introduction

This document describes the binary network protocol used for communication between the R-Type game client and server. The protocol is designed for real-time multiplayer gameplay over UDP with optional TCP for critical messages.

### 1.1 Goals
- **Low latency**: UDP-based for real-time game updates
- **Compact**: Binary encoding to minimize bandwidth
- **Reliable critical messages**: Optional TCP channel or UDP ACK for connection/death events
- **Extensible**: Easy to add new message types

### 1.2 Transport Layer
- **Primary**: UDP on port 4242 (configurable)
- **Optional**: TCP on port 4243 for reliable control messages (lobby, authentication)

---

## 2. Message Format

All messages follow this structure:

```
+--------+--------+----------+----------+---------+
| Magic  | MsgID  | Sequence | Length   | Payload |
| (2B)   | (1B)   | (2B)     | (2B)     | (var)   |
+--------+--------+----------+----------+---------+
```

### 2.1 Header Fields

| Field    | Size (bytes) | Type     | Description                                    |
|----------|--------------|----------|------------------------------------------------|
| Magic    | 2            | uint16   | Protocol magic number: 0x5254 ('RT')           |
| MsgID    | 1            | uint8    | Message type identifier (see section 3)        |
| Sequence | 2            | uint16   | Sequence number for ordering/deduplication     |
| Length   | 2            | uint16   | Payload length in bytes (max 1400)             |
| Payload  | variable     | bytes    | Message-specific data                          |

**Total header size: 7 bytes**

### 2.2 Endianness
All multi-byte fields are in **network byte order (big-endian)**.

### 2.3 Maximum Message Size
- UDP packet max size: **1472 bytes** (to avoid IP fragmentation on typical MTU 1500)
- Max payload: **1400 bytes** (1472 - 7 header - 65 UDP header)

---

## 3. Message Types

### 3.1 Message ID Table

| ID   | Name                | Direction      | Transport | Description                          |
|------|---------------------|----------------|-----------|--------------------------------------|
| 0x01 | CONNECT_REQUEST     | Client → Server| TCP/UDP   | Initial connection handshake         |
| 0x02 | CONNECT_RESPONSE    | Server → Client| TCP/UDP   | Connection accepted/rejected         |
| 0x03 | DISCONNECT          | Both           | TCP/UDP   | Graceful disconnection               |
| 0x10 | INPUT               | Client → Server| UDP       | Player input (movement, shoot)       |
| 0x11 | SNAPSHOT            | Server → Client| UDP       | World state update                   |
| 0x20 | SPAWN_ENTITY        | Server → Client| UDP       | New entity created                   |
| 0x21 | DESTROY_ENTITY      | Server → Client| UDP       | Entity destroyed                     |
| 0x22 | ENTITY_UPDATE       | Server → Client| UDP       | Entity position/state update         |
| 0x30 | PLAYER_DEATH        | Server → Client| TCP/UDP   | Player died (critical)               |
| 0x31 | GAME_OVER           | Server → Client| TCP/UDP   | Game ended                           |
| 0xFF | PING                | Both           | UDP       | Keepalive / latency measurement      |

---

## 4. Message Payloads

### 4.1 CONNECT_REQUEST (0x01)

Client initiates connection.

```
+---------------+---------------+
| Protocol Ver  | Player Name   |
| (1B)          | (32B, UTF-8)  |
+---------------+---------------+
```

- **Protocol Ver**: Protocol version (current: 1)
- **Player Name**: Null-padded UTF-8 string (max 32 bytes)

### 4.2 CONNECT_RESPONSE (0x02)

Server accepts or rejects connection.

```
+--------+-------------+
| Status | Player ID   |
| (1B)   | (4B)        |
+--------+-------------+
```

- **Status**: 0 = Rejected, 1 = Accepted
- **Player ID**: Assigned player entity ID (uint32)

### 4.3 INPUT (0x10)

Client sends player input (sent every frame if input changes).

```
+-------+-------+-------+-------+
| Keys  | MouseX| MouseY| Flags |
| (1B)  | (2B)  | (2B)  | (1B)  |
+-------+-------+-------+-------+
```

- **Keys**: Bitfield (bit 0=Up, 1=Down, 2=Left, 3=Right, 4=Fire, 5=Special)
- **MouseX/Y**: Cursor position (int16, relative to screen)
- **Flags**: Reserved (0)

### 4.4 SNAPSHOT (0x11)

Server sends world state (sent at fixed rate, e.g., 20 Hz).

```
+----------+-----------+---------+
| Timestamp| Num Ents  | Entities|
| (4B)     | (2B)      | (var)   |
+----------+-----------+---------+
```

- **Timestamp**: Server game tick (uint32)
- **Num Ents**: Number of entities in snapshot (uint16)
- **Entities**: Array of entity data (see 4.5)

### 4.5 Entity Data (in SNAPSHOT)

Each entity:

```
+--------+------+------+------+------+------+
| EntID  | Type | X    | Y    | VX   | VY   |
| (4B)   | (1B) | (2B) | (2B) | (2B) | (2B) |
+--------+------+------+------+------+------+
```

- **EntID**: Entity unique ID (uint32)
- **Type**: Entity type (0=Player, 1=Enemy, 2=Missile, 3=Obstacle, etc.)
- **X, Y**: Position in world coordinates (int16, fixed-point or pixel)
- **VX, VY**: Velocity (int16, optional, can be 0)

**Size per entity: 13 bytes**

### 4.6 SPAWN_ENTITY (0x20)

Server notifies entity creation.

```
+--------+------+------+------+
| EntID  | Type | X    | Y    |
| (4B)   | (1B) | (2B) | (2B) |
+--------+------+------+------+
```

### 4.7 DESTROY_ENTITY (0x21)

Server notifies entity destruction.

```
+--------+
| EntID  |
| (4B)   |
+--------+
```

### 4.8 PLAYER_DEATH (0x30)

Critical message: player died.

```
+--------+--------+
| Player | Killer |
| (4B)   | (4B)   |
+--------+--------+
```

- **Player**: ID of dead player
- **Killer**: ID of entity that killed (0 if environment)

### 4.9 PING (0xFF)

Keepalive and latency measurement.

```
+-------------+
| Timestamp   |
| (8B)        |
+-------------+
```

- **Timestamp**: Client send time (uint64, microseconds since epoch)
- Response: Server echoes same timestamp back

---

## 5. Security Considerations

### 5.1 Input Validation
- **Bounds checking**: All coordinates, lengths, and counts must be validated
- **Magic number**: Reject packets with invalid magic (0x5254)
- **Sequence check**: Discard old/duplicate packets (sliding window)

### 5.2 Rate Limiting
- Max INPUT messages per client: 60/sec
- Max connection attempts per IP: 10/min

### 5.3 Malformed Packets
- Server MUST NOT crash on invalid messages
- Log suspicious activity and disconnect abusive clients

---

## 6. Future Extensions

- **Compression**: Optional zlib payload compression (flag in header)
- **Encryption**: TLS wrapper for TCP, DTLS for UDP
- **Delta encoding**: Snapshot compression (only send changed entities)
- **Reliability layer**: Custom ACK/retransmission for UDP critical messages

---

## 7. Example Packet

**CLIENT → SERVER: INPUT (0x10)**

```
Hex dump:
52 54 | 10 | 00 01 | 00 06 | 0F 00 64 00 C8 00

Decoded:
Magic:    0x5254 (RT)
MsgID:    0x10 (INPUT)
Sequence: 0x0001 (1)
Length:   0x0006 (6 bytes)
Payload:
  Keys:   0x0F (Up + Down + Left + Right)
  MouseX: 0x0064 (100)
  MouseY: 0x00C8 (200)
  Flags:  0x00
```

---

## Appendix A: Error Codes

| Code | Name                  | Description                        |
|------|-----------------------|------------------------------------|
| 0x00 | OK                    | Success                            |
| 0x01 | INVALID_PROTOCOL      | Protocol version mismatch          |
| 0x02 | SERVER_FULL           | Max players reached                |
| 0x03 | INVALID_NAME          | Player name rejected               |
| 0x04 | TIMEOUT               | Client didn't respond              |

---

## Appendix B: Changelog

- **v1.0 (2025-11-18)**: Initial draft

---

**End of Document**
