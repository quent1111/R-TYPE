#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace RType {

class SerializationException : public std::runtime_error {
public:
    explicit SerializationException(const std::string& message)
        : std::runtime_error(message) {}
};

class BinarySerializer {
public:
    BinarySerializer() : read_position_(0) {
        buffer_.reserve(256);
    }

    explicit BinarySerializer(const std::vector<uint8_t>& data)
        : buffer_(data), read_position_(0) {}

    explicit BinarySerializer(std::vector<uint8_t>&& data)
        : buffer_(std::move(data)), read_position_(0) {}


    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T>, BinarySerializer&>
    operator<<(const T& value) {
        const size_t old_size = buffer_.size();
        buffer_.resize(old_size + sizeof(T));
        std::memcpy(buffer_.data() + old_size, &value, sizeof(T));
        return *this;
    }

    BinarySerializer& operator<<(const std::string& str) {
        const uint32_t size = static_cast<uint32_t>(str.size());
        *this << size;

        const size_t old_size = buffer_.size();
        buffer_.resize(old_size + size);
        std::memcpy(buffer_.data() + old_size, str.data(), size);

        return *this;
    }

    BinarySerializer& write_bytes(const void* data, size_t size) {
        const size_t old_size = buffer_.size();
        buffer_.resize(old_size + size);
        std::memcpy(buffer_.data() + old_size, data, size);
        return *this;
    }


    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T>, BinarySerializer&>
    operator>>(T& value) {
        if (read_position_ + sizeof(T) > buffer_.size()) {
            throw SerializationException(
                "Buffer underflow: trying to read " + std::to_string(sizeof(T)) +
                " bytes at position " + std::to_string(read_position_) +
                " (buffer size: " + std::to_string(buffer_.size()) + ")"
            );
        }

        std::memcpy(&value, buffer_.data() + read_position_, sizeof(T));
        read_position_ += sizeof(T);
        return *this;
    }

    BinarySerializer& operator>>(std::string& str) {
        uint32_t size = 0;
        *this >> size;

        if (size > 10 * 1024 * 1024) {
            throw SerializationException(
                "String size too large: " + std::to_string(size) + " bytes"
            );
        }

        if (read_position_ + size > buffer_.size()) {
            throw SerializationException(
                "Buffer underflow: trying to read string of " + std::to_string(size) +
                " bytes at position " + std::to_string(read_position_) +
                " (buffer size: " + std::to_string(buffer_.size()) + ")"
            );
        }

        str.assign(
            reinterpret_cast<const char*>(buffer_.data() + read_position_),
            size
        );
        read_position_ += size;

        return *this;
    }

    BinarySerializer& read_bytes(void* data, size_t size) {
        if (read_position_ + size > buffer_.size()) {
            throw SerializationException(
                "Buffer underflow: trying to read " + std::to_string(size) +
                " bytes at position " + std::to_string(read_position_) +
                " (buffer size: " + std::to_string(buffer_.size()) + ")"
            );
        }

        std::memcpy(data, buffer_.data() + read_position_, size);
        read_position_ += size;
        return *this;
    }


    const std::vector<uint8_t>& data() const { return buffer_; }

    std::vector<uint8_t>& data() { return buffer_; }

    const uint8_t* raw_data() const { return buffer_.data(); }

    size_t size() const { return buffer_.size(); }

    size_t read_position() const { return read_position_; }

    size_t remaining() const {
        return buffer_.size() > read_position_ ? buffer_.size() - read_position_ : 0;
    }

    void reset_read_position() { read_position_ = 0; }

    void clear() {
        buffer_.clear();
        read_position_ = 0;
    }

    void reserve(size_t capacity) {
        buffer_.reserve(capacity);
    }

    bool can_read(size_t size) const {
        return read_position_ + size <= buffer_.size();
    }

private:
    std::vector<uint8_t> buffer_;
    size_t read_position_;
};

}
