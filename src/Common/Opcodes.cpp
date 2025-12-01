#include "Opcodes.hpp"

namespace RType {

std::string opcode_to_string(OpCode opcode) {
    switch (opcode) {
        case OpCode::Login:         return "Login";
        case OpCode::Input:         return "Input";
        case OpCode::EntitySpawn:   return "EntitySpawn";
        case OpCode::EntityDestroy: return "EntityDestroy";
        case OpCode::EntityPosition: return "EntityPosition";
        case OpCode::MagicByte1:    return "MagicByte1";
        case OpCode::MagicByte2:    return "MagicByte2";
        default:                    return "Unknown(0x" +
                                           std::to_string(static_cast<int>(opcode)) + ")";
    }
}

}
