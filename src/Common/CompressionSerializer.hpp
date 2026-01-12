#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <lz4.h>
#include <lz4hc.h>
#include "QuantizedSerializer.hpp"

namespace RType {

/**
 * @brief Exception for compression errors
 */
class CompressionException : public std::runtime_error {
public:
    explicit CompressionException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Compression configuration
 */
struct CompressionConfig {
    // Minimum size threshold for compression (bytes)
    // Packets smaller than this won't be compressed (overhead not worth it)
    size_t min_compress_size = 128;

    // LZ4 acceleration factor (1-65537, higher = faster but lower ratio)
    // 1 = best compression, 10 = good balance, 65537 = fastest
    int acceleration = 10;

    // High compression mode (slower but better ratio, for large packets)
    bool use_high_compression = false;

    // Compression level for HC mode (1-12, higher = better compression)
    int hc_level = 9;
};

/**
 * @brief Serializer with LZ4 compression support
 *
 * Extends QuantizedSerializer to add transparent compression/decompression.
 * Uses LZ4 for fast real-time compression suitable for game networking.
 *
 * Features:
 * - Automatic compression above size threshold
 * - Transparent decompression
 * - Compression statistics tracking
 * - Support for both standard and high-compression modes
 *
 * Packet Format (compressed):
 * [1 byte: COMPRESSED_FLAG] [4 bytes: uncompressed_size] [N bytes: compressed_data]
 *
 * Packet Format (uncompressed):
 * [1 byte: UNCOMPRESSED_FLAG] [N bytes: original_data]
 */
class CompressionSerializer : public QuantizedSerializer {
public:
    static constexpr uint8_t UNCOMPRESSED_FLAG = 0x00;
    static constexpr uint8_t COMPRESSED_FLAG = 0x01;

    CompressionSerializer() : QuantizedSerializer(), config_() {}

    explicit CompressionSerializer(const std::vector<uint8_t>& data)
        : QuantizedSerializer(data), config_() {}

    explicit CompressionSerializer(std::vector<uint8_t>&& data)
        : QuantizedSerializer(std::move(data)), config_() {}

    /**
     * @brief Set compression configuration
     */
    void set_config(const CompressionConfig& cfg) {
        config_ = cfg;
    }

    const CompressionConfig& get_config() const {
        return config_;
    }

    // ========================================================================
    // COMPRESSION METHODS
    // ========================================================================

    /**
     * @brief Compress current buffer in-place
     *
     * Only compresses if:
     * - Buffer size >= min_compress_size
     * - Compression actually reduces size
     *
     * @return true if compression was applied, false if left uncompressed
     */
    bool compress() {
        std::vector<uint8_t>& buffer = data();

        if (buffer.size() < config_.min_compress_size) {
            // Prepend uncompressed flag
            buffer.insert(buffer.begin(), UNCOMPRESSED_FLAG);
            return false;
        }

        // Calculate max compressed size
        int max_compressed = LZ4_compressBound(static_cast<int>(buffer.size()));
        std::vector<uint8_t> compressed(static_cast<std::size_t>(max_compressed));

        // Compress
        int compressed_size;
        if (config_.use_high_compression) {
            compressed_size = LZ4_compress_HC(
                reinterpret_cast<const char*>(buffer.data()),
                reinterpret_cast<char*>(compressed.data()),
                static_cast<int>(buffer.size()),
                max_compressed,
                config_.hc_level
            );
        } else {
            compressed_size = LZ4_compress_fast(
                reinterpret_cast<const char*>(buffer.data()),
                reinterpret_cast<char*>(compressed.data()),
                static_cast<int>(buffer.size()),
                max_compressed,
                config_.acceleration
            );
        }

        if (compressed_size <= 0) {
            throw CompressionException("LZ4 compression failed");
        }

        // Only use compression if it actually saves space (+ overhead)
        size_t compressed_total = 1 + 4 + static_cast<std::size_t>(compressed_size); // flag + size + data
        if (compressed_total >= buffer.size()) {
            // Not worth compressing
            buffer.insert(buffer.begin(), UNCOMPRESSED_FLAG);

            // Update stats
            stats_.total_uncompressed++;
            stats_.total_bytes_in += buffer.size() - 1;
            stats_.total_bytes_out += buffer.size() - 1;

            return false;
        }

        // Build compressed packet: [flag][original_size][compressed_data]
        uint32_t original_size = static_cast<uint32_t>(buffer.size());

        std::vector<uint8_t> result;
        result.reserve(compressed_total);
        result.push_back(COMPRESSED_FLAG);

        // Write original size (little-endian)
        result.push_back(static_cast<uint8_t>(original_size & 0xFF));
        result.push_back(static_cast<uint8_t>((original_size >> 8) & 0xFF));
        result.push_back(static_cast<uint8_t>((original_size >> 16) & 0xFF));
        result.push_back(static_cast<uint8_t>((original_size >> 24) & 0xFF));

        // Append compressed data
        result.insert(result.end(), compressed.begin(), compressed.begin() + compressed_size);

        // Replace buffer
        buffer = std::move(result);

        // Update stats
        stats_.total_compressed++;
        stats_.total_bytes_in += original_size;
        stats_.total_bytes_out += buffer.size();

        return true;
    }

    /**
     * @brief Decompress current buffer in-place
     * 
     * Automatically detects if buffer is compressed based on flag.
     * If uncompressed, just removes the flag byte.
     * 
     * @return true if decompression was performed, false if was uncompressed
     */
    bool decompress() {
        std::vector<uint8_t>& buffer = data();

        if (buffer.empty()) {
            throw CompressionException("Cannot decompress empty buffer");
        }

        uint8_t flag = buffer[0];

        if (flag == UNCOMPRESSED_FLAG) {
            // Remove flag byte
            buffer.erase(buffer.begin());
            return false;
        }

        if (flag != COMPRESSED_FLAG) {
            throw CompressionException("Invalid compression flag: " + std::to_string(flag));
        }

        if (buffer.size() < 6) { // flag + size (4 bytes) + at least 1 byte data
            throw CompressionException("Compressed buffer too small");
        }

        // Read original size (little-endian)
        uint32_t original_size =
            static_cast<uint32_t>(buffer[1]) |
            (static_cast<uint32_t>(buffer[2]) << 8) |
            (static_cast<uint32_t>(buffer[3]) << 16) |
            (static_cast<uint32_t>(buffer[4]) << 24);

        if (original_size == 0 || original_size > 1024 * 1024) { // Sanity check: max 1MB
            throw CompressionException("Invalid original size: " + std::to_string(original_size));
        }

        // Decompress
        std::vector<uint8_t> decompressed(original_size);
        int decompressed_size = LZ4_decompress_safe(
            reinterpret_cast<const char*>(buffer.data() + 5), // Skip flag + size
            reinterpret_cast<char*>(decompressed.data()),
            static_cast<int>(buffer.size() - 5),
            static_cast<int>(original_size)
        );

        if (decompressed_size < 0) {
            throw CompressionException("LZ4 decompression failed (corrupted data?)");
        }

        if (static_cast<uint32_t>(decompressed_size) != original_size) {
            throw CompressionException(
                "Decompressed size mismatch: expected " + std::to_string(original_size) +
                ", got " + std::to_string(decompressed_size)
            );
        }

        // Replace buffer with decompressed data
        buffer = std::move(decompressed);

        return true;
    }

    // ========================================================================
    // STATISTICS
    // ========================================================================

    struct CompressionStats {
        size_t total_compressed = 0;      // Number of packets compressed
        size_t total_uncompressed = 0;    // Number of packets left uncompressed
        size_t total_bytes_in = 0;        // Total bytes before compression
        size_t total_bytes_out = 0;       // Total bytes after compression

        double get_compression_ratio() const {
            if (total_bytes_in == 0) return 1.0;
            return static_cast<double>(total_bytes_out) / static_cast<double>(total_bytes_in);
        }

        double get_savings_percent() const {
            if (total_bytes_in == 0) return 0.0;
            return (1.0 - get_compression_ratio()) * 100.0;
        }

        void reset() {
            total_compressed = 0;
            total_uncompressed = 0;
            total_bytes_in = 0;
            total_bytes_out = 0;
        }
    };

    const CompressionStats& get_stats() const {
        return stats_;
    }

    void reset_stats() {
        stats_.reset();
    }

    // ========================================================================
    // CONVENIENCE METHODS
    // ========================================================================

    /**
     * @brief Compress and get final buffer for sending
     */
    const std::vector<uint8_t>& compress_and_data() {
        compress();
        return data();
    }

    /**
     * @brief Create from compressed data and decompress
     */
    static CompressionSerializer from_compressed(const std::vector<uint8_t>& compressed_data) {
        CompressionSerializer serializer(compressed_data);
        serializer.decompress();
        return serializer;
    }

private:
    CompressionConfig config_;
    CompressionStats stats_;
};

} // namespace RType
