#pragma once

#include "zipper_iterator.hpp"

#include <cstddef>

#include <algorithm>
#include <tuple>

namespace containers {

template <class... Containers>
class zipper {
public:
    using iterator = zipper_iterator<Containers...>;
    using iterator_tuple = typename iterator::iterator_tuple;

    explicit zipper(Containers&... cs)
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

}  // namespace containers
