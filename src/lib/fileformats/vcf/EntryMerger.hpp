#pragma once

#include "AlleleMerger.hpp"
#include "common/namespaces.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class CustomType;
class CustomValue;
class Entry;
class Header;
class MergeStrategy;
class SampleData;

class EntryMerger {
public:
    typedef std::map<std::string, CustomValue> CustomValueMap;

    // do these entries overlap? i.e., do we want to merge them?
    static bool canMerge(Entry const& a, Entry const& b);

    EntryMerger(
        MergeStrategy const& mergeStrategy,
        Header const* mergedHeader,
        Entry const* begin,
        Entry const* end);

    // was anything actually merged?
    bool merged() const;
    size_t entryCount() const;
    Entry const* entries() const;

    std::string const& chrom() const;
    uint64_t pos() const;
    std::set<std::string>& identifiers();
    std::string const& ref() const;
    std::set<std::string>& failedFilters();
    double qual() const;
    void setInfo(CustomValueMap& info) const;
    void setAltAndGenotypeData(std::vector<std::string>& alt, SampleData& sampleData) const;

    const Header* mergedHeader() const;

    // The number of times each sample was seen while merging.
    // Used for consensus filtering.
    std::vector<size_t> const& sampleCounts() const;

protected:
    size_t addAllele(const std::string& allele);
    int getPrimaryEntryIdx(std::string const& sampleName) const;

protected:
    AlleleMerger _alleleMerger;
    MergeStrategy const& _mergeStrategy;
    Header const* _mergedHeader;
    Entry const* _begin;
    Entry const* _end;
    double _qual;
    std::set<std::string> _identifiers;
    std::set<std::string> _filters;
    std::set<std::string> _sampleNames;
    std::set<std::string> _info;
    mutable std::vector<size_t> _sampleCounts;
};

END_NAMESPACE(Vcf)
