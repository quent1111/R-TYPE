#pragma once

#include "Entity.hpp"

#include <cstdint>

#include <map>
#include <vector>

namespace GameToNetwork {
enum class MessageType : uint8_t { SendInput, SendLogin, Disconnect, SendReady };

struct Message {
    MessageType type;
    uint8_t input_mask;
    bool ready_status;

    Message(MessageType t, uint8_t input = 0) : type(t), input_mask(input), ready_status(false) {}
    Message(MessageType t, bool ready) : type(t), input_mask(0), ready_status(ready) {}
};
}

namespace NetworkToGame {
enum class MessageType : uint8_t { EntityUpdate, ConnectionStatus, LobbyStatus, StartGame };

struct Message {
    MessageType type;
    std::map<uint32_t, Entity> entities;
    bool is_connected;
    int total_players;
    int ready_players;

    Message(MessageType t) : type(t), is_connected(false), total_players(0), ready_players(0) {}

    Message(MessageType t, std::map<uint32_t, Entity> ents)
        : type(t), entities(std::move(ents)), is_connected(false), total_players(0), ready_players(0) {}
};
}
