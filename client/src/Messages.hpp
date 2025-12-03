#pragma once

#include "Entity.hpp"

#include <cstdint>

#include <map>
#include <vector>

namespace GameToNetwork {
enum class MessageType : uint8_t { SendInput, SendLogin, Disconnect };

struct Message {
    MessageType type;
    uint8_t input_mask;

    Message(MessageType t, uint8_t input = 0) : type(t), input_mask(input) {}
};
}

namespace NetworkToGame {
enum class MessageType : uint8_t { EntityUpdate, ConnectionStatus };

struct Message {
    MessageType type;
    std::map<uint32_t, Entity> entities;
    bool is_connected;

    Message(MessageType t) : type(t), is_connected(false) {}

    Message(MessageType t, std::map<uint32_t, Entity> ents)
        : type(t), entities(std::move(ents)), is_connected(false) {}
};
}
