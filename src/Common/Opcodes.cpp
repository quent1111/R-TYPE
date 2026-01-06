#include "Opcodes.hpp"

namespace RType {

std::string opcode_to_string(OpCode opcode) {
    switch (opcode) {
        case OpCode::Login:         return "Login";
        case OpCode::LoginAck:      return "LoginAck";
        case OpCode::Input:         return "Input";
        case OpCode::EntitySpawn:   return "EntitySpawn";
        case OpCode::EntityDestroy: return "EntityDestroy";
        case OpCode::EntityPosition: return "EntityPosition";
        case OpCode::PlayerReady:   return "PlayerReady";
        case OpCode::LobbyStatus:   return "LobbyStatus";
        case OpCode::StartGame:     return "StartGame";
        case OpCode::ListLobbies:   return "ListLobbies";
        case OpCode::CreateLobby:   return "CreateLobby";
        case OpCode::JoinLobby:     return "JoinLobby";
        case OpCode::LeaveLobby:    return "LeaveLobby";
        case OpCode::LobbyJoined:   return "LobbyJoined";
        case OpCode::LobbyLeft:     return "LobbyLeft";
        case OpCode::LevelStart:    return "LevelStart";
        case OpCode::LevelComplete: return "LevelComplete";
        case OpCode::GameOver:      return "GameOver";
        case OpCode::MagicByte1:    return "MagicByte1";
        case OpCode::MagicByte2:    return "MagicByte2";
        default:                    return "Unknown(0x" +
                                           std::to_string(static_cast<int>(opcode)) + ")";
    }
}

}
