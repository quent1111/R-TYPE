#pragma once

#include "BinarySerializer.hpp"
#include "Opcodes.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace RType::Examples {

std::vector<uint8_t> create_login_packet(const std::string& player_name);
std::string parse_login_packet(const std::vector<uint8_t>& data);

struct InputFlags {
    static constexpr uint8_t MOVE_UP    = 0b00000001;
    static constexpr uint8_t MOVE_DOWN  = 0b00000010;
    static constexpr uint8_t MOVE_LEFT  = 0b00000100;
    static constexpr uint8_t MOVE_RIGHT = 0b00001000;
    static constexpr uint8_t FIRE       = 0b00010000;
    static constexpr uint8_t SPECIAL    = 0b00100000;
};

std::vector<uint8_t> create_input_packet(uint8_t input_flags, uint32_t timestamp);
bool parse_input_packet(const std::vector<uint8_t>& data,
                        uint8_t& out_flags,
                        uint32_t& out_timestamp);

std::vector<uint8_t> create_entity_spawn_packet(uint32_t entity_id,
                                                RType::EntityType entity_type,
                                                float x, float y);
bool parse_entity_spawn_packet(const std::vector<uint8_t>& data,
                               uint32_t& out_entity_id,
                               RType::EntityType& out_type,
                               float& out_x, float& out_y);

std::vector<uint8_t> create_entity_destroy_packet(uint32_t entity_id);
bool parse_entity_destroy_packet(const std::vector<uint8_t>& data,
                                 uint32_t& out_entity_id);

struct EntityPosition {
    uint32_t entity_id;
    float x;
    float y;
};

std::vector<uint8_t> create_batch_position_packet(const std::vector<EntityPosition>& positions);
bool parse_batch_position_packet(const std::vector<uint8_t>& data,
                                 std::vector<EntityPosition>& out_positions);

}
