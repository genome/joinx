#pragma once

#include "io/StreamHandler.hpp"
#include "fileformats/vcf/Entry.hpp"
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
        std::ostream& out,
        std::string const& outputDir);

    void operator()(
            size_t sampleIdx,
            std::string const& sequence,
            Vcf::RawVariant::Vector const& vars,
            std::map<size_t, Vcf::Entry const*> const& which
            );

    void finalize();

    std::vector<size_t> orderedCountsForSample(size_t sampleIdx) const;

protected:
    std::string trimmedBinaryString(size_t x) const;
    std::string makeFilePath(size_t sampleIdx, std::set<size_t> const& which) const;
    std::ostream& getOutputFile(size_t sampleIdx, std::set<size_t> const& which);

private:
    size_t nStreams_;
    std::vector<std::string> const& fileNames_;
    std::vector<std::string> const& sampleNames_;
    std::vector<
        boost::unordered_map<std::set<size_t>, size_t>
        > counts_;
    std::ostream& reportOut_;
    std::string const& outputDir_;
    StreamHandler streams_;
    std::set<std::string> openPaths_;
};
