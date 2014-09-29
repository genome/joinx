#pragma once

#include "common/RelOps.hpp"
#include "common/compat.hpp"

#include <memory>
#include <vector>

template<
          typename OutputFunc
        , typename Compare
        >
class GroupSorter {
public:
    GroupSorter(OutputFunc& out, Compare cmp = Compare())
        : out(out)
        , cmp(cmp)
    {}

    template<typename ValuePtr>
    void operator()(std::vector<ValuePtr> entries) {
        DerefBinaryOp<Compare> dcmp(cmp);
        compat::sort(entries.begin(), entries.end(), dcmp);
        out(std::move(entries));
    }

    OutputFunc& out;
    Compare cmp;
};

template<
          typename OutputFunc
        , typename Compare
        >
GroupSorter<OutputFunc, Compare>
makeGroupSorter(OutputFunc& out, Compare cmp = Compare()) {
    return GroupSorter<OutputFunc, Compare>(out, cmp);
}


