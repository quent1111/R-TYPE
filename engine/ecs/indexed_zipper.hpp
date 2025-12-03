#pragma once

#include <cstddef>

#include <tuple>
#include <utility>

namespace containers {

template <class... Containers>
class indexed_zipper;

template <class... Containers>
class indexed_zipper_iterator {
    template <class Container>
    using iterator_t = decltype(std::declval<Container&>().begin());

    template <class Container>
    using it_reference_t = typename iterator_t<Container>::reference;

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
    using value_type =
        std::tuple<std::size_t, typename extract_value_type<it_reference_t<Containers>>::type...>;
    using reference = value_type;
    using pointer = void;
    using difference_type = std::size_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_tuple = std::tuple<iterator_t<Containers>...>;

    friend class indexed_zipper<Containers...>;

private:
    explicit indexed_zipper_iterator(iterator_tuple const& it_tuple, std::size_t max)
        : _current(it_tuple), _max(max), _idx(0) {
        if (_idx < _max && !all_set(_seq)) {
            advance_to_next_valid();
        }
    }

    explicit indexed_zipper_iterator(iterator_tuple const& it_tuple, std::size_t max,
                                     std::size_t idx)
        : _current(it_tuple), _max(max), _idx(idx) {}

public:
    indexed_zipper_iterator(indexed_zipper_iterator const& z) = default;

    indexed_zipper_iterator& operator++() {
        if (_idx < _max) {
            increment_iterators(_seq);
            advance_to_next_valid();
        }
        return *this;
    }

    indexed_zipper_iterator operator++(int) {
        indexed_zipper_iterator tmp = *this;
        if (_idx < _max) {
            increment_iterators(_seq);
            advance_to_next_valid();
        }
        return tmp;
    }

    value_type operator*() { return to_value(_seq); }

    value_type operator->() { return to_value(_seq); }

    friend bool operator==(indexed_zipper_iterator const& lhs, indexed_zipper_iterator const& rhs) {
        return lhs._idx == rhs._idx;
    }

    friend bool operator!=(indexed_zipper_iterator const& lhs, indexed_zipper_iterator const& rhs) {
        return !(lhs == rhs);
    }

private:
    template <std::size_t... Is>
    void increment_iterators(std::index_sequence<Is...>) {
        (++std::get<Is>(_current), ...);
        ++_idx;
    }

    void advance_to_next_valid() {
        while (_idx < _max && !all_set(_seq)) {
            increment_iterators(_seq);
        }
    }

    template <std::size_t... Is>
    bool all_set(std::index_sequence<Is...>) {
        if (_idx >= _max)
            return false;
        return ((*std::get<Is>(_current)).has_value() && ...);
    }

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

    explicit indexed_zipper(Containers&... cs)
        : _begin(std::make_tuple(cs.begin()...)),
          _end(_compute_end(cs...)),
          _size(_compute_size(cs...)) {}

    iterator begin() { return iterator(_begin, _size); }

    iterator end() { return iterator(_end, _size, _size); }

private:
    static std::size_t _compute_size(Containers&... containers) {
        std::size_t min_size = static_cast<std::size_t>(-1);
        ((min_size = std::min(min_size, containers.size())), ...);
        return min_size == static_cast<std::size_t>(-1) ? 0 : min_size;
    }

    static iterator_tuple _compute_end(Containers&... containers) {
        return std::make_tuple(containers.end()...);
    }

private:
    iterator_tuple _begin;
    iterator_tuple _end;
    std::size_t _size;
};

}
