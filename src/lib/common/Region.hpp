#pragma once

#include "common/cstdint.hpp"

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

    int64_t overlap(Region const& that) const;
};
