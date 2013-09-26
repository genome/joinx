#pragma once

#include "common/cstdint.hpp"
#include "common/namespaces.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

class Entry;

class AlleleMerger {
public:
    typedef std::vector< std::vector<size_t> > AltIndices;
    typedef std::map<std::string, size_t> AlleleMap;

    static std::string buildRef(Entry const* beg, Entry const* end);

    AlleleMerger(Entry const* beg, Entry const* end);
    AlleleMerger(std::vector<Entry> const& ents);

    bool merged() const { return _merged; }
    std::string const& ref() const { return _ref; }
    std::vector<std::string> const& mergedAlt() const { return _mergedAlt; }
    AltIndices const& newAltIndices() const { return _newAltIndices; }

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
    AltIndices _newAltIndices;
};

END_NAMESPACE(Vcf)
