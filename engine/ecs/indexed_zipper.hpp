#pragma once

#include <cstddef>

#include <tuple>
#include <utility>

namespace containers {

template <class... Containers>
class indexed_zipper;

template <class... Containers>
class indexed_zipper_iterator {
    // Type alias for the iterator type of a container
    template <class Container>
    using iterator_t = decltype(std::declval<Container&>().begin());

    // Type alias for the reference type of an iterator
    template <class Container>
    using it_reference_t = typename iterator_t<Container>::reference;

    // Helper to extract the value type from std::optional<T>& -> T&
    template <class T>
    struct extract_value_type;

    template <class T>
    struct extract_value_type<std::optional<T>&> {
        using type = T&;
    };

    template <class T>
    struct extract_value_type<const std::optional<T>&> {
        using type = const T&;
    };

public:
    // value_type is a tuple with index first, then references to components
    using value_type =
        std::tuple<std::size_t, typename extract_value_type<it_reference_t<Containers>>::type...>;
    using reference = value_type;
    using pointer = void;
    using difference_type = std::size_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_tuple = std::tuple<iterator_t<Containers>...>;

    // Make indexed_zipper a friend so it can construct us
    friend class indexed_zipper<Containers...>;

private:
    // Private constructor - only indexed_zipper can create us
    explicit indexed_zipper_iterator(iterator_tuple const& it_tuple, std::size_t max)
        : _current(it_tuple), _max(max), _idx(0) {
        // If we're not at the end and current position is not valid, find the first valid one
        if (_idx < _max && !all_set(_seq)) {
            advance_to_next_valid();
        }
    }

    // Constructor for end iterator
    explicit indexed_zipper_iterator(iterator_tuple const& it_tuple, std::size_t max,
                                     std::size_t idx)
        : _current(it_tuple), _max(max), _idx(idx) {}

public:
    // Copy constructor
    indexed_zipper_iterator(indexed_zipper_iterator const& z) = default;

    // Pre-increment: ++it
    indexed_zipper_iterator& operator++() {
        if (_idx < _max) {
            increment_iterators(_seq);
            advance_to_next_valid();
        }
        return *this;
    }

    // Post-increment: it++
    indexed_zipper_iterator operator++(int) {
        indexed_zipper_iterator tmp = *this;
        if (_idx < _max) {
            increment_iterators(_seq);
            advance_to_next_valid();
        }
        return tmp;
    }

    // Dereference operator - returns tuple with index
    value_type operator*() { return to_value(_seq); }

    // Arrow operator
    value_type operator->() { return to_value(_seq); }

    // Equality comparison
    friend bool operator==(indexed_zipper_iterator const& lhs, indexed_zipper_iterator const& rhs) {
        return lhs._idx == rhs._idx;
    }

    // Inequality comparison
    friend bool operator!=(indexed_zipper_iterator const& lhs, indexed_zipper_iterator const& rhs) {
        return !(lhs == rhs);
    }

private:
    // Increment all iterators by one
    template <std::size_t... Is>
    void increment_iterators(std::index_sequence<Is...>) {
        (++std::get<Is>(_current), ...);
        ++_idx;
    }

    // Advance to the next valid position (where all components are set)
    void advance_to_next_valid() {
        while (_idx < _max && !all_set(_seq)) {
            increment_iterators(_seq);
        }
    }

    // Check if all std::optional at current position are set (have a value)
    template <std::size_t... Is>
    bool all_set(std::index_sequence<Is...>) {
        if (_idx >= _max)
            return false;
        return ((*std::get<Is>(_current)).has_value() && ...);
    }

    // Convert current iterator positions to a tuple with index and component references
    template <std::size_t... Is>
    value_type to_value(std::index_sequence<Is...>) {
        return std::make_tuple(_idx, std::ref((*std::get<Is>(_current)).value())...);
    }

private:
    iterator_tuple _current;
    std::size_t _max;
    std::size_t _idx;
    static constexpr std::index_sequence_for<Containers...> _seq{};
};

template <class... Containers>
class indexed_zipper {
public:
    using iterator = indexed_zipper_iterator<Containers...>;
    using iterator_tuple = typename iterator::iterator_tuple;

    // Constructor takes references to all containers
    explicit indexed_zipper(Containers&... cs)
        : _begin(std::make_tuple(cs.begin()...)),
          _end(_compute_end(cs...)),
          _size(_compute_size(cs...)) {}

    // Return iterator to beginning
    iterator begin() { return iterator(_begin, _size); }

    // Return iterator to end
    iterator end() { return iterator(_end, _size, _size); }

private:
    // Compute the maximum size (smallest non-empty container)
    static std::size_t _compute_size(Containers&... containers) {
        std::size_t min_size = static_cast<std::size_t>(-1);
        ((min_size = std::min(min_size, containers.size())), ...);
        return min_size == static_cast<std::size_t>(-1) ? 0 : min_size;
    }

    // Compute end iterator tuple
    static iterator_tuple _compute_end(Containers&... containers) {
        return std::make_tuple(containers.end()...);
    }

private:
    iterator_tuple _begin;
    iterator_tuple _end;
    std::size_t _size;
};

}  // namespace containers
