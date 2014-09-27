#pragma once

#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <memory>
#include <vector>

template<typename OutputFunc>
class VcfReheaderer {
public:
    typedef std::unique_ptr<Vcf::Entry> ValuePtr;
    typedef std::vector<ValuePtr> ValuePtrVector;

    VcfReheaderer(OutputFunc& out, Vcf::Header const* header)
        : out_(out)
        , header_(header)
    {}

    void operator()(ValuePtrVector entries) {
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            (*i)->reheader(header_);
        }
        out_(std::move(entries));
    }

private:
    OutputFunc& out_;
    Vcf::Header const* header_;
};

template<typename OutputFunc>
VcfReheaderer<OutputFunc>
makeVcfReheaderer(OutputFunc& out, Vcf::Header const* header) {
    return VcfReheaderer<OutputFunc>(out, header);
}
