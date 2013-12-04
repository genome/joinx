#pragma once

#include "fileformats/vcf/RawVariant.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <cstddef>
#include <ostream>
#include <set>
#include <string>
#include <vector>

class VcfCompareGt {
public:
    VcfCompareGt(
        std::vector<std::string> const& fileNames,
        std::vector<std::string> const& sampleNames,
        std::ostream& out);

    ~VcfCompareGt();

    void operator()(
            size_t sampleIdx,
            std::string const& sequence,
            Vcf::RawVariant::Vector const& vars,
            std::set<size_t> const& which
            );

    void finalize();

    std::vector<size_t> orderedCountsForSample(size_t sampleIdx) const;

private:
    class Impl;
    boost::scoped_ptr<Impl> impl_;
};
