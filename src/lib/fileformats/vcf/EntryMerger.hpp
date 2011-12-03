#pragma once

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

class EntryMerger {
public:
    typedef std::map<std::string, CustomValue> CustomValueMap;
    typedef std::map<std::string, uint32_t> AlleleMap;
    EntryMerger(const MergeStrategy& mergeStrategy, const Header* mergedHeader, const Entry* begin, const Entry* end);

    const std::string& chrom() const;
    uint64_t pos() const;
    const std::set<std::string>& identifiers() const;
    const std::string& ref() const;
    const AlleleMap& alleleMap() const;
    const std::set<std::string>& failedFilters() const;
    double qual() const;
    void setInfo(CustomValueMap& info) const;
    void setAltAndGenotypeData(
        std::vector<std::string>& alt,
        std::vector<CustomType const*>& format,
        std::map< uint32_t, std::vector<CustomValue> >& sampleData) const;

    const Header* mergedHeader() const;

protected:
    size_t addAllele(const std::string& allele);

protected:
    const MergeStrategy& _mergeStrategy;
    const Header* _mergedHeader;
    const Entry* _begin;
    const Entry* _end;
    const Entry* _refEntry;
    double _qual;
    std::set<std::string> _identifiers;
    AlleleMap _alleleMap;
    AlleleMap::size_type _alleleIdx;
    std::set<std::string> _filters;
    std::set<std::string> _sampleNames;
    std::set<std::string> _info;

    std::vector< std::vector< size_t > > _newGTIndices;
};

END_NAMESPACE(Vcf)
