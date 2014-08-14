#pragma once

#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/GenotypeDictionary.hpp"
// FIXME: we're borrowing the FilterType enum from here
// eventually, this may replace GenotypeComparator
#include "fileformats/vcf/GenotypeComparator.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "io/StreamHandler.hpp"

#include <boost/function.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>
#include <set>
#include <vector>

class VcfGenotypeMatcher {
public:
    typedef size_t FileIndex;
    typedef Vcf::GenotypeDictionary<FileIndex> GenotypeDict;

    typedef std::vector<std::unique_ptr<Vcf::Entry>> EntryList;
    typedef std::vector<Vcf::RawVariant::Vector> SampleGenotypes;
    typedef std::vector<SampleGenotypes> EntryGenotypes;
    typedef boost::unordered_map<std::set<FileIndex>, size_t> SampleCounter;

    typedef boost::function<void(Vcf::Entry const&)> EntryOutput;

    VcfGenotypeMatcher(
          std::vector<std::string> const& streamNames
        , std::vector<std::string> const& sampleNames
        , std::string const& exactFieldName
        , std::string const& partialFieldName
        , std::vector<Vcf::FilterType> const& filterTypes
        , EntryOutput& entryOutput
        );

    void operator()(EntryList&& entries);

    // Add the file indices of partial matches found in dict for the alleles
    // in genotype to the set partials.
    boost::container::flat_set<FileIndex> partialMatchingFiles(
          GenotypeDict const& dict
        , Vcf::RawVariant::Vector const& genotype
        );

    bool shouldSkip(size_t streamIdx, bool isFiltered) const;

    void collectEntry(size_t entryIdx);
    void annotateEntry(size_t idx);

    void updateCounts();
    void reset();

    void writeEntries() const;
    void reportCounts(std::ostream& os) const;

private:
    uint32_t const numFiles_;
    uint32_t const numSamples_;
    std::string const& exactFieldName_;
    std::string const& partialFieldName_;
    std::vector<std::string> const& streamNames_;
    std::vector<std::string> const& sampleNames_;
    std::vector<Vcf::FilterType> const& filterTypes_;
    EntryOutput& entryOutput_;

    EntryList entries_;
    std::vector<GenotypeDict> gtDicts_;
    EntryGenotypes entryGenotypes_;
    std::vector<SampleCounter> sampleCounters_;
};
