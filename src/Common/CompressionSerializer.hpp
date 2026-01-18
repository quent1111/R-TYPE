#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <lz4.h>
#include <lz4hc.h>
#include "QuantizedSerializer.hpp"

namespace RType {


class CompressionException : public std::runtime_error {
public:
    explicit CompressionException(const std::string& message)
        : std::runtime_error(message) {}
};


struct CompressionConfig {
    size_t min_compress_size = 128;

    int acceleration = 10;

    bool use_high_compression = false;

    int hc_level = 9;
};

class CompressionSerializer : public QuantizedSerializer {
public:
    static constexpr uint8_t UNCOMPRESSED_FLAG = 0x00;
    static constexpr uint8_t COMPRESSED_FLAG = 0x01;

    CompressionSerializer() : QuantizedSerializer(), config_() {}

    explicit CompressionSerializer(const std::vector<uint8_t>& data)
        : QuantizedSerializer(data), config_() {}

    explicit CompressionSerializer(std::vector<uint8_t>&& data)
        : QuantizedSerializer(std::move(data)), config_() {}

    void set_config(const CompressionConfig& cfg) {
        config_ = cfg;
    }

    const CompressionConfig& get_config() const {
        return config_;
    }


    bool compress() {
        std::vector<uint8_t>& buffer = data();

        if (buffer.size() < config_.min_compress_size) {
            buffer.insert(buffer.begin(), UNCOMPRESSED_FLAG);
            return false;
        }

        int max_compressed = LZ4_compressBound(static_cast<int>(buffer.size()));
        std::vector<uint8_t> compressed(static_cast<std::size_t>(max_compressed));

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

        size_t compressed_total = 1 + 4 + static_cast<std::size_t>(compressed_size);
        if (compressed_total >= buffer.size()) {
            buffer.insert(buffer.begin(), UNCOMPRESSED_FLAG);

            stats_.total_uncompressed++;
            stats_.total_bytes_in += buffer.size() - 1;
            stats_.total_bytes_out += buffer.size() - 1;

            return false;
        }

        uint32_t original_size = static_cast<uint32_t>(buffer.size());

        std::vector<uint8_t> result;
        result.reserve(compressed_total);
        result.push_back(COMPRESSED_FLAG);

        result.push_back(static_cast<uint8_t>(original_size & 0xFF));
        result.push_back(static_cast<uint8_t>((original_size >> 8) & 0xFF));
        result.push_back(static_cast<uint8_t>((original_size >> 16) & 0xFF));
        result.push_back(static_cast<uint8_t>((original_size >> 24) & 0xFF));

        result.insert(result.end(), compressed.begin(), compressed.begin() + compressed_size);

        buffer = std::move(result);

        stats_.total_compressed++;
        stats_.total_bytes_in += original_size;
        stats_.total_bytes_out += buffer.size();

        return true;
    }

    bool decompress() {
        std::vector<uint8_t>& buffer = data();

        if (buffer.empty()) {
            throw CompressionException("Cannot decompress empty buffer");
        }

        uint8_t flag = buffer[0];

        if (flag == UNCOMPRESSED_FLAG) {
            buffer.erase(buffer.begin());
            return false;
        }

        if (flag != COMPRESSED_FLAG) {
            throw CompressionException("Invalid compression flag: " + std::to_string(flag));
        }

        if (buffer.size() < 6) {
            throw CompressionException("Compressed buffer too small");
        }

        uint32_t original_size =
            static_cast<uint32_t>(buffer[1]) |
            (static_cast<uint32_t>(buffer[2]) << 8) |
            (static_cast<uint32_t>(buffer[3]) << 16) |
            (static_cast<uint32_t>(buffer[4]) << 24);

        if (original_size == 0 || original_size > 1024 * 1024) {
            throw CompressionException("Invalid original size: " + std::to_string(original_size));
        }

        std::vector<uint8_t> decompressed(original_size);
        int decompressed_size = LZ4_decompress_safe(
            reinterpret_cast<const char*>(buffer.data() + 5),
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

        buffer = std::move(decompressed);

        return true;
    }


    struct CompressionStats {
        size_t total_compressed = 0;
        size_t total_uncompressed = 0;
        size_t total_bytes_in = 0;
        size_t total_bytes_out = 0;

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

    const std::vector<uint8_t>& compress_and_data() {
        compress();
        return data();
    }

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
