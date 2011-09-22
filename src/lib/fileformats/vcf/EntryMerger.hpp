#pragma once

#include "namespace.hpp"
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

VCF_NAMESPACE_BEGIN

class CustomValue;
class Entry;
class Header;
class MergeStrategy;

class EntryMerger {
public:
    typedef std::map<std::string, CustomValue> CustomValueMap;
    typedef std::map<std::string, uint32_t> AlleleMap;
    EntryMerger(const MergeStrategy& mergeStrategy, const Header* mergedHeader, const Entry* begin, const Entry* end);

    const std::string& chrom() const;
    uint64_t pos() const;
    const std::set<std::string> identifiers() const;
    const std::string& ref() const;
    const AlleleMap& alleleMap() const;
    const std::set<std::string> failedFilters() const;
    double qual() const;
    void setInfo(CustomValueMap& info) const;
    void setGenotypeData(
        std::vector<std::string>& format,
        std::vector< std::vector<CustomValue> >& genotypeData) const; 

protected:
    const MergeStrategy& _mergeStrategy;
    const Header* _mergedHeader;
    const Entry* _begin;
    const Entry* _end;
    std::set<std::string> _identifiers;
    AlleleMap _alleleMap;
    std::set<std::string> _filters;
    std::set<std::string> _sampleNames;
    std::set<std::string> _info;
};

VCF_NAMESPACE_END
