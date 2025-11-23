#pragma once

#include <cstddef>

// Forward declaration
class registry;

// Entity class with private constructor - only registry can create entities
class entity {
    friend class registry;

public:
    // Conversion operators
    constexpr explicit operator std::size_t() const noexcept { return _id; }
    // Comparison operators
    constexpr bool operator==(entity const& other) const noexcept { return _id == other._id; }
    constexpr bool operator!=(entity const& other) const noexcept { return _id != other._id; }
    constexpr bool operator<(entity const& other) const noexcept { return _id < other._id; }
    // Get the raw ID
    constexpr std::size_t id() const noexcept { return _id; }

private:
    // Private constructor - only registry can create entities via friend
    constexpr explicit entity(std::size_t id) noexcept : _id(id) {}
    std::size_t _id;
};
