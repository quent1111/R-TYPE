#include "PacketExamples.hpp"

namespace RType::Examples {

std::vector<uint8_t> create_login_packet(const std::string& player_name) {
    BinarySerializer serializer;
    serializer << MagicNumber::VALUE;
    serializer << OpCode::Login;
    serializer << player_name;
    return serializer.data();
}

std::string parse_login_packet(const std::vector<uint8_t>& data) {
    try {
        BinarySerializer deserializer(data);
        uint16_t magic;
        deserializer >> magic;
        if (!MagicNumber::is_valid(magic)) {
            std::cerr << "[Login] Invalid magic number: 0x"
                      << std::hex << magic << std::dec << std::endl;
            return "";
        }
        OpCode opcode;
        deserializer >> opcode;
        if (opcode != OpCode::Login) {
            std::cerr << "[Login] Invalid opcode: "
                      << opcode_to_string(opcode) << std::endl;
            return "";
        }
        std::string player_name;
        deserializer >> player_name;
        return player_name;
    } catch (const SerializationException& e) {
        std::cerr << "[Login] Deserialization error: " << e.what() << std::endl;
        return "";
    }
}

std::vector<uint8_t> create_input_packet(uint8_t input_flags, uint32_t timestamp) {
    BinarySerializer serializer;
    serializer << MagicNumber::VALUE;
    serializer << OpCode::Input;
    serializer << input_flags;
    serializer << timestamp;
    return serializer.data();
}

bool parse_input_packet(const std::vector<uint8_t>& data,
                        uint8_t& out_flags,
                        uint32_t& out_timestamp) {
    try {
        BinarySerializer deserializer(data);
        uint16_t magic;
        deserializer >> magic;
        if (!MagicNumber::is_valid(magic)) {
            return false;
        }
        OpCode opcode;
        deserializer >> opcode;
        if (opcode != OpCode::Input) {
            return false;
        }
        deserializer >> out_flags >> out_timestamp;
        return true;
    } catch (const SerializationException& e) {
        std::cerr << "[Input] Parse error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<uint8_t> create_entity_spawn_packet(uint32_t entity_id,
                                                RType::EntityType entity_type,
                                                float x, float y) {
    BinarySerializer serializer;
    serializer << MagicNumber::VALUE;
    serializer << OpCode::EntitySpawn;
    serializer << entity_id;
    serializer << entity_type;
    serializer << x;
    serializer << y;
    return serializer.data();
}

bool parse_entity_spawn_packet(const std::vector<uint8_t>& data,
                               uint32_t& out_entity_id,
                               RType::EntityType& out_type,
                               float& out_x, float& out_y) {
    try {
        BinarySerializer deserializer(data);
        uint16_t magic;
        deserializer >> magic;
        if (!MagicNumber::is_valid(magic))
            return false;
        OpCode opcode;
        deserializer >> opcode;
        if (opcode != OpCode::EntitySpawn)
            return false;
        deserializer >> out_entity_id >> out_type >> out_x >> out_y;
        return true;
    } catch (const SerializationException&) {
        return false;
    }
}

std::vector<uint8_t> create_entity_destroy_packet(uint32_t entity_id) {
    BinarySerializer serializer;
    serializer << MagicNumber::VALUE;
    serializer << OpCode::EntityDestroy;
    serializer << entity_id;
    return serializer.data();
}

bool parse_entity_destroy_packet(const std::vector<uint8_t>& data,
                                 uint32_t& out_entity_id) {
    try {
        BinarySerializer deserializer(data);
        uint16_t magic;
        deserializer >> magic;
        if (!MagicNumber::is_valid(magic))
            return false;
        OpCode opcode;
        deserializer >> opcode;
        if (opcode != OpCode::EntityDestroy)
            return false;
        deserializer >> out_entity_id;
        return true;
    } catch (const SerializationException&) {
        return false;
    }
}

std::vector<uint8_t> create_batch_position_packet(const std::vector<EntityPosition>& positions) {
    BinarySerializer serializer;
    serializer.reserve(6 + positions.size() * 12);
    serializer << MagicNumber::VALUE;
    serializer << OpCode::EntityPosition;
    serializer << static_cast<uint8_t>(positions.size());
    for (const auto& pos : positions) {
        serializer << pos.entity_id << pos.x << pos.y;
    }
    return serializer.data();
}

bool parse_batch_position_packet(const std::vector<uint8_t>& data,
                                 std::vector<EntityPosition>& out_positions) {
    try {
        BinarySerializer deserializer(data);
        uint16_t magic;
        deserializer >> magic;
        if (!MagicNumber::is_valid(magic))
            return false;
        OpCode opcode;
        deserializer >> opcode;
        if (opcode != OpCode::EntityPosition)
            return false;
        uint8_t count;
        deserializer >> count;
        out_positions.clear();
        out_positions.reserve(count);
        for (uint8_t i = 0; i < count; ++i) {
            EntityPosition pos;
            deserializer >> pos.entity_id >> pos.x >> pos.y;
            out_positions.push_back(pos);
        }
        return true;
    } catch (const SerializationException& e) {
        std::cerr << "[BatchPosition] Parse error: " << e.what() << std::endl;
        return false;
    }
}

}
