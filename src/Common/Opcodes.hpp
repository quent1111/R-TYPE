#pragma once

#include <cstdint>
#include <string>

namespace RType {

enum class OpCode : uint8_t {
    Login = 0x01,
    Input = 0x10,
    EntitySpawn = 0x11,
    EntityDestroy = 0x12,
    EntityPosition = 0x13,
    MagicByte1 = 0x42,
    MagicByte2 = 0xB5
};

enum class EntityType : uint8_t {
    Player      = 0x01,
    Enemy       = 0x02,
    Projectile  = 0x03,
    Powerup     = 0x04,
    Obstacle    = 0x05
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
