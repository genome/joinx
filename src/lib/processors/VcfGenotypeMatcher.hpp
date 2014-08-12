#pragma once

#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"

#include <boost/container/flat_set.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
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

    VcfGenotypeMatcher(uint32_t numFiles, uint32_t numSamples);

    void operator()(EntryList&& entries);

    void collectEntry(size_t entryIdx);
    void annotateEntry(size_t idx);

    void getCounts();
    void reset();

    void finalize();

private:
    uint32_t numFiles_;
    uint32_t numSamples_;

    EntryList entries_;
    std::vector<GenotypeDictionary> gtDicts_;

    EntryGenotypes entryGenotypes_;

    std::vector<SampleCounter> sampleCounters_;
};
