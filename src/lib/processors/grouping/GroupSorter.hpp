#pragma once

#include "common/RelOps.hpp"
#include "common/compat.hpp"

#include <memory>
#include <vector>

template<
          typename ValueType
        , typename OutputFunc
        , typename Compare
        >
class GroupSorter {
public:
    GroupSorter(OutputFunc& out, Compare cmp = Compare())
        : out(out)
        , cmp(cmp)
    {}

    typedef std::unique_ptr<ValueType> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    void operator()(ValuePtrVector entries) {
        DerefBinaryOp<Compare> dcmp(cmp);
        compat::sort(entries.begin(), entries.end(), dcmp);
        out(std::move(entries));
    }

    OutputFunc& out;
    Compare cmp;
};

template<
          typename ValueType
        , typename OutputFunc
        , typename Compare
        >
GroupSorter<ValueType, OutputFunc, Compare>
makeGroupSorter(OutputFunc& out, Compare cmp = Compare()) {
    return GroupSorter<ValueType, OutputFunc, Compare>(out, cmp);
}


