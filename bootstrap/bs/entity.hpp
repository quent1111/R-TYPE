#pragma once

#include <cstddef>

// Forward declaration
class registry;

// Entity class with private constructor - only registry can create entities
class entity {
	friend class registry;

public:
	// Conversion operators
	operator std::size_t() const { return _id; }
	// Comparison operators
	bool operator==(entity const& other) const { return _id == other._id; }
	bool operator!=(entity const& other) const { return _id != other._id; }
	bool operator<(entity const& other) const { return _id < other._id; }
	// Get the raw ID
	std::size_t id() const { return _id; }

private:
	// Private constructor - only registry can create entities via friend
	explicit entity(std::size_t id) : _id(id) {}
	std::size_t _id;
};


