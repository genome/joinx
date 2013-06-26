#pragma once

#include "common/namespaces.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class Entry;

class AlleleMerger {
public:
    typedef std::vector< std::vector<size_t> > GtIndices;
    typedef std::map<std::string, size_t> AlleleMap;

    static std::string buildRef(Entry const* beg, Entry const* end);

    AlleleMerger(Entry const* beg, Entry const* end);
    AlleleMerger(std::vector<Entry> const& ents);

    bool merged() const { return _merged; }
    std::string const& ref() const { return _ref; }
    std::vector<std::string> const& mergedAlt() const { return _mergedAlt; }
    GtIndices const& newGt() const { return _newGt; }
    AlleleMap const& alleleMap() const { return _alleleMap; }

protected:
    void init(Entry const* beg, Entry const* end);

protected:
    uint32_t addAllele(std::string const& v);

protected:
    AlleleMap _alleleMap;
    uint32_t _alleleIdx;
    std::string _ref;

    // return values
    bool _merged;
    std::vector<std::string> _mergedAlt;
    GtIndices _newGt;
};

END_NAMESPACE(Vcf)
