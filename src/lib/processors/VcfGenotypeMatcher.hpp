#pragma once

#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/RawVariant.hpp"

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class GenotypeDictionary {
public:
    typedef boost::unordered_map<size_t, size_t> LocationCounts;
    typedef boost::unordered_map<Vcf::RawVariant::Vector, LocationCounts> ExactMapType;
    typedef boost::unordered_map<Vcf::RawVariant, LocationCounts> PartialMapType;

    void add(Vcf::RawVariant const& gt, size_t count, size_t entryIdx);
    LocationCounts const* match(Vcf::RawVariant const& gt) const;
    void clear();

private:
    ExactMapType exactMap_;
    PartialMapType partialMap_;
};

class VcfGenotypeMatcher {
public:
    typedef std::vector<std::unique_ptr<Vcf::Entry>> EntryList;
    typedef boost::unordered_map<Vcf::RawVariant, size_t> AlleleCounts;
    typedef std::vector<AlleleCounts> SampleAlleleCounts;
    typedef std::vector<SampleAlleleCounts> EntryAlleleCounts;

    VcfGenotypeMatcher(uint32_t numFiles, uint32_t numSamples);

    void operator()(EntryList&& entries);

    void collectEntry(size_t entryIdx);
    void annotateEntry(size_t idx);

    void getCounts();
    void reset();

private:
    uint32_t numFiles_;
    uint32_t numSamples_;

    EntryList entries_;
    std::vector<GenotypeDictionary> gtDicts_;
    EntryAlleleCounts entryAlleleCounts_;
};
