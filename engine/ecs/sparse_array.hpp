#pragma once

#include <vector>
#include <optional>
#include <memory>
#include <cstddef>

template <typename Component>
// You can also mirror the definition of std::vector, that takes an additional allocator
class sparse_array {
public:
	using value_type = std::optional<Component>; // optional component type
	using reference_type = value_type&;
	using const_reference_type = value_type const&;
	using container_t = std::vector<value_type>; // optionally add your allocator
	// template here.
	using size_type = typename container_t::size_type;
	using iterator = typename container_t::iterator;
	using const_iterator = typename container_t::const_iterator;

public:
	// defaulted special members
	sparse_array() = default;
	sparse_array(sparse_array const&) = default;
	sparse_array(sparse_array&&) noexcept = default;
	~sparse_array() = default;
	sparse_array& operator=(sparse_array const&) = default;
	sparse_array& operator=(sparse_array&&) noexcept = default;

	// element access - returns optional<Component>
	reference_type operator[](size_t idx)
	{
		if (idx >= _data.size())
			_data.resize(idx + 1);
		return _data[idx];
	}

	const_reference_type operator[](size_t idx) const
	{
		if (idx >= _data.size())
			return _null_optional;
		return _data[idx];
	}

	// iterators
	iterator begin() noexcept { return _data.begin(); }
	const_iterator begin() const noexcept { return _data.begin(); }
	const_iterator cbegin() const noexcept { return _data.cbegin(); }

	iterator end() noexcept { return _data.end(); }
	const_iterator end() const noexcept { return _data.end(); }
	const_iterator cend() const noexcept { return _data.cend(); }

	// capacity
	size_type size() const noexcept { return _data.size(); }

	// insert at position (lvalue) - returns reference to the optional
	reference_type insert_at(size_type pos, Component const& value)
	{
		if (pos >= _data.size())
			_data.resize(pos + 1);
		_data[pos] = value;
		return _data[pos];
	}

	// insert at position (rvalue) - returns reference to the optional
	reference_type insert_at(size_type pos, Component&& value)
	{
		if (pos >= _data.size())
			_data.resize(pos + 1);
		_data[pos] = std::move(value);
		return _data[pos];
	}

	// emplace at position - uses allocator to destroy and reconstruct in-place
	template <class... Params>
	reference_type emplace_at(size_type pos, Params&&... params)
	{
		if (pos >= _data.size())
			_data.resize(pos + 1);

		auto& slot = _data[pos];
		// If there's already a component, we need to destroy and reconstruct it
		if (slot.has_value()) {
			// Get the allocator and traits for the Component type
			using component_allocator = typename std::allocator_traits<typename container_t::allocator_type>::template rebind_alloc<Component>;
			using component_alloc_traits = std::allocator_traits<component_allocator>;
			component_allocator alloc = _data.get_allocator();
			// Destroy the existing component
			component_alloc_traits::destroy(alloc, std::addressof(slot.value()));
			// Construct new component in-place
			component_alloc_traits::construct(alloc, std::addressof(slot.value()), std::forward<Params>(params)...);
		} else {
			// No existing component, just emplace
			slot.emplace(std::forward<Params>(params)...);
		}
		return slot;
	}

	void erase(size_type pos)
	{
		if (pos < _data.size())
			_data[pos].reset(); // Set to std::nullopt
	}

	// get_index using std::addressof to find by address, not by value comparison
	size_type get_index(value_type const& v) const noexcept
	{
		// Find by address comparison - we're looking for the address of the optional itself
		const void* target_addr = std::addressof(v);
		for (size_type i = 0; i < _data.size(); ++i) {
			if (std::addressof(_data[i]) == target_addr)
				return i;
		}
		return static_cast<size_type>(-1);
	}

private:
	container_t _data;
	static inline const value_type _null_optional = std::nullopt;
};