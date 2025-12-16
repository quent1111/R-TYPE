#pragma once

#include "game/Entity.hpp"

#include <cstdint>

#include <map>
#include <vector>

namespace GameToNetwork {
enum class MessageType : uint8_t {
    SendInput,
    SendLogin,
    Disconnect,
    SendReady,
    SendWeaponUpgrade,
    SendPowerUpChoice,
    SendPowerUpActivate
};

struct Message {
    MessageType type;
    uint8_t input_mask;
    bool ready_status;
    uint8_t weapon_upgrade_choice;
    uint8_t powerup_choice_value;

    Message(MessageType t, uint8_t input = 0)
        : type(t),
          input_mask(input),
          ready_status(false),
          weapon_upgrade_choice(0),
          powerup_choice_value(0) {}
    Message(MessageType t, bool ready)
        : type(t),
          input_mask(0),
          ready_status(ready),
          weapon_upgrade_choice(0),
          powerup_choice_value(0) {}

    static Message weapon_upgrade(uint8_t choice) {
        Message msg(MessageType::SendWeaponUpgrade);
        msg.weapon_upgrade_choice = choice;
        return msg;
    }

    static Message powerup_choice(uint8_t choice) {
        Message msg(MessageType::SendPowerUpChoice);
        msg.powerup_choice_value = choice;
        return msg;
    }
    static Message powerup_activate() { return Message(MessageType::SendPowerUpActivate); }
};
}  // namespace GameToNetwork

namespace NetworkToGame {
enum class MessageType : uint8_t {
    EntityUpdate,
    ConnectionStatus,
    LobbyStatus,
    StartGame,
    LevelProgress,
    LevelComplete,
    LevelStart,
    PowerUpSelection,
    PowerUpStatus,
    GameOver
};

struct Message {
    MessageType type;
    std::map<uint32_t, Entity> entities;
    uint32_t my_network_id;
    bool is_connected;
    int total_players;
    int ready_players;
    uint32_t level;
    uint32_t kills;
    uint32_t enemies_needed;
    uint32_t powerup_player_id;
    uint8_t powerup_type;
    float powerup_time_remaining;
    bool show_powerup_selection;
    uint8_t current_level;
    uint16_t enemies_killed;
    uint8_t next_level;

    Message(MessageType t)
        : type(t),
          my_network_id(0),
          is_connected(false),
          total_players(0),
          ready_players(0),
          level(1),
          kills(0),
          enemies_needed(0),
          powerup_player_id(0),
          powerup_type(0),
          powerup_time_remaining(0.0f),
          show_powerup_selection(false),
          current_level(1),
          enemies_killed(0),
          next_level(1) {}

    Message(MessageType t, std::map<uint32_t, Entity> ents)
        : type(t),
          entities(std::move(ents)),
          my_network_id(0),
          is_connected(false),
          total_players(0),
          ready_players(0),
          level(1),
          kills(0),
          enemies_needed(0),
          powerup_type(0),
          powerup_time_remaining(0.0f),
          show_powerup_selection(false),
          current_level(1),
          enemies_killed(0),
          next_level(1) {}
    static Message level_progress(uint32_t lvl, uint32_t killed, uint32_t needed) {
        Message msg(MessageType::LevelProgress);
        msg.level = lvl;
        msg.kills = killed;
        msg.enemies_needed = needed;
        msg.current_level = static_cast<uint8_t>(lvl);
        msg.enemies_killed = static_cast<uint16_t>(killed);
        return msg;
    }
    static Message level_complete(uint32_t completed, uint32_t next) {
        Message msg(MessageType::LevelComplete);
        msg.level = completed;
        msg.current_level = static_cast<uint8_t>(completed);
        msg.next_level = static_cast<uint8_t>(next);
        return msg;
    }
    static Message level_start(uint32_t lvl) {
        Message msg(MessageType::LevelStart);
        msg.level = lvl;
        msg.current_level = static_cast<uint8_t>(lvl);
        return msg;
    }
};
}  // namespace NetworkToGame
