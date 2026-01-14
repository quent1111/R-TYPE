#pragma once

#include <cstdint>
#include <string>

namespace RType {

enum class OpCode : uint8_t {
    Login = 0x01,
    LoginAck = 0x02,
    Input = 0x10,
    EntitySpawn = 0x11,
    EntityDestroy = 0x12,
    EntityPosition = 0x13,
    PlayerReady = 0x20,
    LobbyStatus = 0x21,
    StartGame = 0x22,
    ListLobbies = 0x23,
    CreateLobby = 0x24,
    JoinLobby = 0x25,
    LeaveLobby = 0x26,
    LobbyJoined = 0x27,
    LobbyLeft = 0x28,
    LevelStart = 0x30,
    LevelComplete = 0x31,
    WeaponUpgradeChoice = 0x32,
    LevelProgress = 0x33,
    PowerUpChoice = 0x34,
    PowerUpActivate = 0x35,
    PowerUpStatus = 0x36,
    PowerUpCards = 0x37,
    ActivableSlots = 0x38,
    BossSpawn = 0x50,
    GameOver = 0x40,
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
    Enemy4       = 0x0E,
    Enemy5       = 0x0F,
    HomingEnemy  = 0x09,
    Ally         = 0x0A,
    LaserBeam    = 0x0B,
    SupportDrone = 0x0C,
    MissileDrone = 0x0D,
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
    CompilerBoss = 0x20,
    CompilerPart1 = 0x21,
    CompilerPart2 = 0x22,
    CompilerPart3 = 0x23
};

struct MagicNumber {
    static constexpr uint16_t VALUE = 0xB542;
    static constexpr uint8_t BYTE1 = static_cast<uint8_t>(OpCode::MagicByte1);
    static constexpr uint8_t BYTE2 = static_cast<uint8_t>(OpCode::MagicByte2);

    static bool is_valid(uint16_t value) {
        return value == VALUE;
    }

    static bool is_valid(uint8_t byte1, uint8_t byte2) {
        return byte1 == BYTE1 && byte2 == BYTE2;
    }
};

std::string opcode_to_string(OpCode opcode);

}
