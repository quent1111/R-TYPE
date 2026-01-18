#pragma once

#include <cstdint>
#include <cmath>
#include <vector>
#include <stdexcept>
#include "BinarySerializer.hpp"

namespace RType {

class QuantizedSerializer : public BinarySerializer {
public:
    QuantizedSerializer() : BinarySerializer() {}

    explicit QuantizedSerializer(const std::vector<uint8_t>& data)
        : BinarySerializer(data) {}

    explicit QuantizedSerializer(std::vector<uint8_t>&& data)
        : BinarySerializer(std::move(data)) {}

    QuantizedSerializer& write_quantized_position(float value) {
        if (value < 0.0f) value = 0.0f;
        if (value > 6553.5f) value = 6553.5f;
        uint16_t quantized = static_cast<uint16_t>(value * 10.0f);
        *this << quantized;
        return *this;
    }

    float read_quantized_position() {
        uint16_t quantized;
        *this >> quantized;
        return static_cast<float>(quantized) / 10.0f;
    }

    QuantizedSerializer& write_position(float x, float y) {
        write_quantized_position(x);
        write_quantized_position(y);
        return *this;
    }

    void read_position(float& x, float& y) {
        x = read_quantized_position();
        y = read_quantized_position();
    }

    QuantizedSerializer& write_quantized_velocity(float value) {
        if (value < -1270.0f) value = -1270.0f;
        if (value > 1270.0f) value = 1270.0f;
        int8_t quantized = static_cast<int8_t>(value / 10.0f);
        *this << quantized;
        return *this;
    }
    float read_quantized_velocity() {
        int8_t quantized;
        *this >> quantized;
        return static_cast<float>(quantized) * 10.0f;
    }
    QuantizedSerializer& write_velocity(float vx, float vy) {
        write_quantized_velocity(vx);
        write_quantized_velocity(vy);
        return *this;
    }
    void read_velocity(float& vx, float& vy) {
        vx = read_quantized_velocity();
        vy = read_quantized_velocity();
    }

    QuantizedSerializer& write_quantized_angle(float degrees) {
        while (degrees < 0.0f) degrees += 360.0f;
        while (degrees >= 360.0f) degrees -= 360.0f;
        uint8_t quantized = static_cast<uint8_t>((degrees / 360.0f) * 255.0f);
        *this << quantized;
        return *this;
    }
    float read_quantized_angle() {
        uint8_t quantized;
        *this >> quantized;
        return (static_cast<float>(quantized) / 255.0f) * 360.0f;
    }
    QuantizedSerializer& write_quantized_health(int current, int maximum) {
        if (maximum <= 255) {
            *this << static_cast<uint8_t>(current);
            *this << static_cast<uint8_t>(maximum);
        } else {
            uint8_t percentage = static_cast<uint8_t>((current * 100) / maximum);
            *this << percentage;
            *this << static_cast<uint8_t>(100);
        }
        return *this;
    }
    void read_quantized_health(int& current, int& maximum) {
        uint8_t curr, max;
        *this >> curr >> max;
        current = curr;
        maximum = max;
    }
    QuantizedSerializer& write_packed_flags(const bool* flags, size_t count) {
        if (count > 8) {
            throw SerializationException("Cannot pack more than 8 flags in one byte");
        }

        uint8_t packed = 0;
        for (size_t i = 0; i < count; ++i) {
            if (flags[i]) {
                packed |= (1 << i);
            }
        }
        *this << packed;
        return *this;
    }

    void read_packed_flags(bool* flags, size_t count) {
        if (count > 8) {
            throw SerializationException("Cannot unpack more than 8 flags from one byte");
        }

        uint8_t packed;
        *this >> packed;

        for (size_t i = 0; i < count; ++i) {
            flags[i] = (packed & (1 << i)) != 0;
        }
    }

    QuantizedSerializer& write_entity_transform(float x, float y, float vx, float vy) {
        write_position(x, y);
        write_velocity(vx, vy);
        return *this;
    }

    void read_entity_transform(float& x, float& y, float& vx, float& vy) {
        read_position(x, y);
        read_velocity(vx, vy);
    }

    static constexpr float get_position_compression_ratio() {
        return 0.5f;
    }

    static constexpr float get_velocity_compression_ratio() {
        return 0.25f;
    }

    static constexpr float get_transform_compression_ratio() {
        return 0.375f;
    }

  
    template<typename T>
    QuantizedSerializer& operator<<(const T& value) {
        BinarySerializer::operator<<(value);
        return *this;
    }
};

struct EntityFlags {
    bool is_shooting      : 1;
    bool has_shield       : 1;
    bool has_powerup      : 1;
    bool is_invulnerable  : 1;
    bool is_stunned       : 1;
    bool is_critical_hp   : 1;
    bool reserved1        : 1;
    bool reserved2        : 1;

    EntityFlags()
        : is_shooting(false)
        , has_shield(false)
        , has_powerup(false)
        , is_invulnerable(false)
        , is_stunned(false)
        , is_critical_hp(false)
        , reserved1(false)
        , reserved2(false) {}

    uint8_t pack() const {
        return (is_shooting      ? 0x01 : 0) |
               (has_shield       ? 0x02 : 0) |
               (has_powerup      ? 0x04 : 0) |
               (is_invulnerable  ? 0x08 : 0) |
               (is_stunned       ? 0x10 : 0) |
               (is_critical_hp   ? 0x20 : 0) |
               (reserved1        ? 0x40 : 0) |
               (reserved2        ? 0x80 : 0);
    }

    void unpack(uint8_t packed) {
        is_shooting      = (packed & 0x01) != 0;
        has_shield       = (packed & 0x02) != 0;
        has_powerup      = (packed & 0x04) != 0;
        is_invulnerable  = (packed & 0x08) != 0;
        is_stunned       = (packed & 0x10) != 0;
        is_critical_hp   = (packed & 0x20) != 0;
        reserved1        = (packed & 0x40) != 0;
        reserved2        = (packed & 0x80) != 0;
    }
};

} // namespace RType
