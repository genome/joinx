#pragma once

#include "common/RelOps.hpp"

#include <memory>
#include <vector>

template<typename OutputFunc>
class GroupForEach {
public:
    explicit GroupForEach(OutputFunc& out)
        : out_(out)
    {}

    template<typename ValuePtr>
    void operator()(std::vector<ValuePtr> entries) {
        for (auto i = entries.begin(); i != entries.end(); ++i)
            out_(std::move(*i));
    }

    OutputFunc& out_;
};

template<typename OutputFunc>
GroupForEach<OutputFunc>
makeGroupForEach(OutputFunc& out) {
    return GroupForEach<OutputFunc>(out);
}
