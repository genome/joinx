#pragma once

#include "fileformats/vcf/RawVariant.hpp"

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

class VcfCompareGt {
public:
    VcfCompareGt(std::vector<std::string> const& sampleNames, std::ostream& out);

    void operator()(
            size_t sampleIdx,
            std::string const& sequence,
            Vcf::RawVariant::Vector const& vars,
            std::vector<size_t> which
            );

    void finalize();

private:
    std::vector<std::string> sampleNames_;
    std::vector<
        boost::unordered_map<std::string, size_t>
        > counts_;
    std::ostream& out_;
};
