#pragma once

#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"
#include "io/StreamHandler.hpp"

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

class GenotypeDictionary {
public:
    typedef size_t EntryIndex;
    typedef size_t Count;

    typedef boost::unordered_map<EntryIndex, Count> LocationCounts;
    typedef boost::container::flat_set<EntryIndex> Locations;

    typedef boost::unordered_map<Vcf::RawVariant::Vector, Locations> ExactMapType;
    typedef boost::unordered_map<Vcf::RawVariant, LocationCounts> PartialMapType;

    void add(Vcf::RawVariant::Vector const& gt, size_t entryIdx);
    void clear();

    Locations const* exactMatches(Vcf::RawVariant::Vector const& gt) const;
    LocationCounts const* allMatches(Vcf::RawVariant const& gt) const;

    PartialMapType copyPartialMatches() const { return partialMap_; }

private:
    ExactMapType exactMap_;
    PartialMapType partialMap_;
};

class VcfGenotypeMatcher {
public:
    typedef std::vector<std::unique_ptr<Vcf::Entry>> EntryList;
    typedef std::vector<Vcf::RawVariant::Vector> SampleGenotypes;
    typedef std::vector<SampleGenotypes> EntryGenotypes;
    typedef boost::unordered_map<std::set<size_t>, size_t> SampleCounter;

    VcfGenotypeMatcher(
          std::vector<std::string> const& streamNames
        , std::vector<std::string> const& sampleNames
        , std::string const& exactFieldName
        , std::string const& partialFieldName
        , std::string const& outputDir
        );

    void operator()(EntryList&& entries);

    void collectEntry(size_t entryIdx);
    void annotateEntry(size_t idx);

    void updateCounts();
    void reset();

    void writeEntries() const;
    void reportCounts(std::ostream& os) const;

    std::ostream& getStream(size_t idx) const;

private:
    uint32_t const numFiles_;
    uint32_t const numSamples_;
    std::string const& exactFieldName_;
    std::string const& partialFieldName_;
    std::vector<std::string> const& sampleNames_;
    std::vector<std::string> const& streamNames_;
    std::string const& outputDir_;

    EntryList entries_;
    std::vector<GenotypeDictionary> gtDicts_;
    EntryGenotypes entryGenotypes_;
    std::vector<SampleCounter> sampleCounters_;

    mutable std::vector<bool> wroteHeader_;
    mutable StreamHandler streams_;
};
