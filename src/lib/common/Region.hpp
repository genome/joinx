#pragma once

#include "common/cstdint.hpp"

#include <boost/functional/hash.hpp>

#include <algorithm>

struct Region {
    int64_t begin;
    int64_t end;


    Region()
        : begin(0)
        , end(0)
    {}

    Region(int64_t begin, int64_t end)
        : begin(begin)
        , end(end)
    {}

    bool contains(Region const& that) const;
    bool isContainedIn(Region const& that) const;

    int64_t overlap(Region const& that) const;
    int64_t size() const;

    bool operator==(Region const& rhs) const {
        return begin == rhs.begin && end == rhs.end;
    }

    bool operator!=(Region const& rhs) const {
        return begin != rhs.begin || end != rhs.end;
    }
};

inline
size_t hash_value(Region const& r) {
    size_t seed = 0;
    boost::hash_combine(seed, r.begin);
    boost::hash_combine(seed, r.end);
    return seed;
}
