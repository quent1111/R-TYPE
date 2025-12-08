#pragma once

#include <cstddef>

class registry;

class entity {
    friend class registry;

public:
    constexpr explicit operator std::size_t() const noexcept { return _id; }
    constexpr bool operator==(entity const& other) const noexcept { return _id == other._id; }
    constexpr bool operator!=(entity const& other) const noexcept { return _id != other._id; }
    constexpr bool operator<(entity const& other) const noexcept { return _id < other._id; }
    constexpr std::size_t id() const noexcept { return _id; }

private:
    constexpr explicit entity(std::size_t id) noexcept : _id(id) {}
    std::size_t _id;
};
