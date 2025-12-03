#pragma once

#include <cstddef>

#include <memory>
#include <optional>
#include <vector>

template <typename Component>
class sparse_array {
public:
    using value_type = std::optional<Component>;
    using reference_type = value_type&;
    using const_reference_type = value_type const&;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

public:
    sparse_array() = default;
    sparse_array(sparse_array const&) = default;
    sparse_array(sparse_array&&) noexcept = default;
    ~sparse_array() = default;
    sparse_array& operator=(sparse_array const&) = default;
    sparse_array& operator=(sparse_array&&) noexcept = default;

    reference_type operator[](size_t idx) {
        if (idx >= _data.size())
            _data.resize(idx + 1);
        return _data[idx];
    }

    const_reference_type operator[](size_t idx) const {
        if (idx >= _data.size())
            return _null_optional;
        return _data[idx];
    }

    iterator begin() noexcept { return _data.begin(); }
    const_iterator begin() const noexcept { return _data.begin(); }
    const_iterator cbegin() const noexcept { return _data.cbegin(); }

    iterator end() noexcept { return _data.end(); }
    const_iterator end() const noexcept { return _data.end(); }
    const_iterator cend() const noexcept { return _data.cend(); }

    size_type size() const noexcept { return _data.size(); }

    reference_type insert_at(size_type pos, Component const& value) {
        if (pos >= _data.size())
            _data.resize(pos + 1);
        _data[pos] = value;
        return _data[pos];
    }

    reference_type insert_at(size_type pos, Component&& value) {
        if (pos >= _data.size())
            _data.resize(pos + 1);
        _data[pos] = std::move(value);
        return _data[pos];
    }

    template <class... Params>
    reference_type emplace_at(size_type pos, Params&&... params) {
        if (pos >= _data.size())
            _data.resize(pos + 1);

        auto& slot = _data[pos];
        if (slot.has_value()) {
            using component_allocator = typename std::allocator_traits<
                typename container_t::allocator_type>::template rebind_alloc<Component>;
            using component_alloc_traits = std::allocator_traits<component_allocator>;
            component_allocator alloc = _data.get_allocator();
            component_alloc_traits::destroy(alloc, std::addressof(slot.value()));
            component_alloc_traits::construct(alloc, std::addressof(slot.value()),
                                              std::forward<Params>(params)...);
        } else {
            slot.emplace(std::forward<Params>(params)...);
        }
        return slot;
    }

    void erase(size_type pos) {
        if (pos < _data.size())
            _data[pos].reset();
    }

    size_type get_index(value_type const& v) const noexcept {
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