#pragma once

#include "common/RelOps.hpp"

#include <memory>
#include <vector>

template<
          typename ValueType
        , typename OutputFunc
        >
class GroupForEach {
public:
    explicit GroupForEach(OutputFunc& out)
        : out_(out)
    {}

    typedef std::unique_ptr<ValueType> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    void operator()(ValuePtrVector entries) {
        for (auto i = entries.begin(); i != entries.end(); ++i)
            out_(std::move(*i));
    }

    OutputFunc& out_;
};

template<
          typename ValueType
        , typename OutputFunc
        >
GroupForEach<ValueType, OutputFunc>
makeGroupForEach(OutputFunc& out) {
    return GroupForEach<ValueType, OutputFunc>(out);
}
