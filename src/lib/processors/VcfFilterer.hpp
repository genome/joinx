#pragma once

#include "fileformats/vcf/Entry.hpp"

#include <memory>
#include <string>
#include <vector>

template<typename OutputFunc>
class VcfFilterer {
public:
    typedef std::unique_ptr<Vcf::Entry> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    VcfFilterer(OutputFunc& out, std::string filterName)
        : out_(out)
        , filterName_(std::move(filterName))
    {}

    void operator()(ValuePtrVector entries) {
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            (*i)->addFilter(filterName_);
        }
        out_(std::move(entries));
    }

private:
    OutputFunc& out_;
    std::string filterName_;
};

template<typename OutputFunc>
VcfFilterer<OutputFunc>
makeVcfFilterer(OutputFunc& out, std::string filterName) {
    return VcfFilterer<OutputFunc>(out, std::move(filterName));
}
