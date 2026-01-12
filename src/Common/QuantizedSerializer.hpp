#pragma once

#include <cstdint>
#include <cmath>
#include <vector>
#include <stdexcept>
#include "BinarySerializer.hpp"

namespace RType {

/**
 * @brief Quantization configuration for different data types
 * 
 * Reduces float precision to use smaller integer types:
 * - Position: float (4 bytes) → uint16 (2 bytes) with 0.1 pixel precision
 * - Velocity: float (4 bytes) → int8 (1 byte) for -128 to 127 range
 * - Angle: float (4 bytes) → uint8 (1 byte) for 0-360° with 1.4° precision
 */
class QuantizedSerializer : public BinarySerializer {
public:
    QuantizedSerializer() : BinarySerializer() {}
    
    explicit QuantizedSerializer(const std::vector<uint8_t>& data)
        : BinarySerializer(data) {}

    explicit QuantizedSerializer(std::vector<uint8_t>&& data)
        : BinarySerializer(std::move(data)) {}
    
    // ========================================================================
    // POSITION QUANTIZATION (16-bit: 0.1 pixel precision, 0-6553.5 range)
    // ========================================================================
    
    /**
     * @brief Write quantized position (X or Y coordinate)
     * @param value Position in pixels (0.0f to 6553.5f)
     * @return Reference to this serializer for chaining
     * 
     * Encoding: float → uint16 (multiply by 10)
     * - Input: 123.45f → Output: 1234 (2 bytes)
     * - Precision: 0.1 pixel
     * - Range: 0 to 6553.5 pixels
     * - Bandwidth: 50% reduction (4 bytes → 2 bytes)
     */
    QuantizedSerializer& write_quantized_position(float value) {
        // Clamp to valid range
        if (value < 0.0f) value = 0.0f;
        if (value > 6553.5f) value = 6553.5f;
        
        // Quantize: multiply by 10 and convert to uint16
        uint16_t quantized = static_cast<uint16_t>(value * 10.0f);
        *this << quantized;
        return *this;
    }
    
    /**
     * @brief Read quantized position
     * @return Dequantized position value
     */
    float read_quantized_position() {
        uint16_t quantized;
        *this >> quantized;
        return static_cast<float>(quantized) / 10.0f;
    }
    
    /**
     * @brief Write quantized position pair (X, Y)
     * @param x X coordinate
     * @param y Y coordinate
     */
    QuantizedSerializer& write_position(float x, float y) {
        write_quantized_position(x);
        write_quantized_position(y);
        return *this;
    }
    
    /**
     * @brief Read quantized position pair
     * @param x Output X coordinate
     * @param y Output Y coordinate
     */
    void read_position(float& x, float& y) {
        x = read_quantized_position();
        y = read_quantized_position();
    }
    
    // ========================================================================
    // VELOCITY QUANTIZATION (8-bit: -127 to 127 range, x10 multiplier)
    // ========================================================================
    
    /**
     * @brief Write quantized velocity (VX or VY)
     * @param value Velocity in pixels/sec (-1270.0f to 1270.0f)
     * 
     * Encoding: float → int8 (divide by 10)
     * - Input: 250.5f → Output: 25 (1 byte)
     * - Precision: 10 pixels/sec
     * - Range: -1270 to 1270 pixels/sec
     * - Bandwidth: 75% reduction (4 bytes → 1 byte)
     */
    QuantizedSerializer& write_quantized_velocity(float value) {
        // Clamp to valid range
        if (value < -1270.0f) value = -1270.0f;
        if (value > 1270.0f) value = 1270.0f;
        
        // Quantize: divide by 10 and convert to int8
        int8_t quantized = static_cast<int8_t>(value / 10.0f);
        *this << quantized;
        return *this;
    }
    
    /**
     * @brief Read quantized velocity
     */
    float read_quantized_velocity() {
        int8_t quantized;
        *this >> quantized;
        return static_cast<float>(quantized) * 10.0f;
    }
    
    /**
     * @brief Write quantized velocity pair (VX, VY)
     */
    QuantizedSerializer& write_velocity(float vx, float vy) {
        write_quantized_velocity(vx);
        write_quantized_velocity(vy);
        return *this;
    }
    
    /**
     * @brief Read quantized velocity pair
     */
    void read_velocity(float& vx, float& vy) {
        vx = read_quantized_velocity();
        vy = read_quantized_velocity();
    }
    
    // ========================================================================
    // ANGLE QUANTIZATION (8-bit: 0-360° with ~1.4° precision)
    // ========================================================================
    
    /**
     * @brief Write quantized angle (0-360 degrees)
     * @param degrees Angle in degrees (0.0f to 360.0f)
     * 
     * Encoding: float → uint8 (multiply by 255/360)
     * - Input: 45.0f → Output: 32 (1 byte)
     * - Precision: 1.41° (360/255)
     * - Range: 0 to 360 degrees
     * - Bandwidth: 75% reduction (4 bytes → 1 byte)
     */
    QuantizedSerializer& write_quantized_angle(float degrees) {
        // Normalize to 0-360 range
        while (degrees < 0.0f) degrees += 360.0f;
        while (degrees >= 360.0f) degrees -= 360.0f;
        
        // Quantize: map 0-360 to 0-255
        uint8_t quantized = static_cast<uint8_t>((degrees / 360.0f) * 255.0f);
        *this << quantized;
        return *this;
    }
    
    /**
     * @brief Read quantized angle
     */
    float read_quantized_angle() {
        uint8_t quantized;
        *this >> quantized;
        return (static_cast<float>(quantized) / 255.0f) * 360.0f;
    }
    
    // ========================================================================
    // HEALTH QUANTIZATION (8-bit: 0-255 HP)
    // ========================================================================
    
    /**
     * @brief Write quantized health (0-255 range)
     * @param current Current HP
     * @param maximum Maximum HP
     * 
     * If HP > 255, stores as percentage (0-100) instead
     */
    QuantizedSerializer& write_quantized_health(int current, int maximum) {
        if (maximum <= 255) {
            // Direct storage
            *this << static_cast<uint8_t>(current);
            *this << static_cast<uint8_t>(maximum);
        } else {
            // Store as percentage
            uint8_t percentage = static_cast<uint8_t>((current * 100) / maximum);
            *this << percentage;
            *this << static_cast<uint8_t>(100); // Max = 100%
        }
        return *this;
    }
    
    /**
     * @brief Read quantized health
     */
    void read_quantized_health(int& current, int& maximum) {
        uint8_t curr, max;
        *this >> curr >> max;
        current = curr;
        maximum = max;
    }
    
    // ========================================================================
    // BIT PACKING FOR FLAGS (1 byte = 8 bools)
    // ========================================================================
    
    /**
     * @brief Pack up to 8 boolean flags into 1 byte
     * @param flags Array of booleans (max 8)
     * @param count Number of flags (1-8)
     * 
     * Example:
     *   bool flags[] = {true, false, true, false};
     *   write_packed_flags(flags, 4);
     *   // Result: 0b00000101 (bit 0 and 2 set)
     */
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
    
    /**
     * @brief Unpack flags from packed byte
     * @param flags Output array of booleans (must have space for count)
     * @param count Number of flags to unpack (1-8)
     */
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
    
    // ========================================================================
    // HELPER: Write common entity data (optimized)
    // ========================================================================
    
    /**
     * @brief Write optimized entity transform data
     * @param x Position X
     * @param y Position Y
     * @param vx Velocity X
     * @param vy Velocity Y
     * 
     * Total: 6 bytes (vs 16 bytes uncompressed = 62.5% reduction)
     */
    QuantizedSerializer& write_entity_transform(float x, float y, float vx, float vy) {
        write_position(x, y);      // 4 bytes (2+2)
        write_velocity(vx, vy);    // 2 bytes (1+1)
        return *this;
    }
    
    /**
     * @brief Read optimized entity transform data
     */
    void read_entity_transform(float& x, float& y, float& vx, float& vy) {
        read_position(x, y);
        read_velocity(vx, vy);
    }
    
    // ========================================================================
    // STATISTICS
    // ========================================================================
    
    /**
     * @brief Calculate compression ratio vs uncompressed float encoding
     */
    static constexpr float get_position_compression_ratio() {
        return 0.5f; // 2 bytes / 4 bytes = 50%
    }
    
    static constexpr float get_velocity_compression_ratio() {
        return 0.25f; // 1 byte / 4 bytes = 25%
    }
    
    static constexpr float get_transform_compression_ratio() {
        return 0.375f; // 6 bytes / 16 bytes = 37.5%
    }
    
    // ========================================================================
    // COMPATIBILITY LAYER - Stream operator for chaining
    // ========================================================================

    /**
     * @brief Stream insertion operator for primitive types
     * Forwards to BinarySerializer's operator<< for compatibility
     */
    template<typename T>
    QuantizedSerializer& operator<<(const T& value) {
        BinarySerializer::operator<<(value);
        return *this;
    }
};

/**
 * @brief Helper struct for entity flags bit-packing
 */
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
    
    /**
     * @brief Pack all flags into single byte
     */
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
    
    /**
     * @brief Unpack from byte
     */
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
