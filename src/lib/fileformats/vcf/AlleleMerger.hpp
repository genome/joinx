#pragma once

#include "Entry.hpp"
#include "common/namespaces.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

BEGIN_NAMESPACE(Vcf)

namespace {
    bool entryPosLess(Entry const& a, Entry const& b) {
        return a.pos() < b.pos();
    }

    bool entryRefStopLess(Entry const& a, Entry const& b) {
        return a.refStop() < b.refStop();
    }

    bool chromEq(const std::string& chrom, Entry const& b) {
        return chrom == b.chrom();
    }
}

template<typename RefSeq>
class AlleleMerger {
public:
    typedef std::vector< std::vector<size_t> > GtIndices;
    typedef std::map<std::string, size_t> AlleleMap;

    AlleleMerger(RefSeq& ref, std::vector<Entry> const& ents);

    bool merged() const { return _merged; }
    vector<string> const& mergedAlt() const { return _mergedAlt; }
    GtIndices const& newGt() const { return _newGt; }

protected:
    uint32_t addAllele(std::string const& v) {
        auto inserted = _alleleMap.insert(make_pair(v, _alleleIdx));
        if (inserted.second)
            ++_alleleIdx;
        return inserted.first->second;
    }
    

protected:
    AlleleMap _alleleMap;
    uint32_t _alleleIdx;

    // return values
    bool _merged;
    std::vector<std::string> _mergedAlt;
    GtIndices _newGt;
};

template<typename RefSeq>
inline AlleleMerger<RefSeq>::AlleleMerger(RefSeq& ref, std::vector<Entry> const& ents)
    : _alleleIdx(0)
    , _merged(false)
{
    using namespace std;
    using namespace std::placeholders;

    if (ents.empty())
        return;

    if (!all_of(ents.begin()+1, ents.end(), bind(&chromEq, ents[0].chrom(), _1))) {
        return;
    }

    uint64_t beg = min_element(ents.begin(), ents.end(), &entryPosLess)->pos();
    uint64_t end = max_element(ents.begin(), ents.end(), &entryRefStopLess)->refStop();
    string seq = ref.sequence(ents[0].chrom(), beg, end);

    _newGt.resize(ents.size());

    size_t inputAlts(0);
    for (auto e = ents.begin(); e != ents.end(); ++e) {
        inputAlts += e->alt().size();
        for (auto alt = e->alt().begin(); alt != e->alt().end(); ++alt) {
            std::string var = seq;
            int64_t off = e->pos() - beg;
            int32_t len = alt->size() - e->ref().size();
            if (len == 0) { // SNV
                for (std::string::size_type i = 0; i < alt->size(); ++i)
                    var[off+i] = (*alt)[i];
                _newGt[e-ents.begin()].push_back(addAllele(var));
            } else if (len < 0) {
                var.erase(off+alt->size(), -len);
                _newGt[distance(ents.begin(), e)].push_back(addAllele(var));
            } else { // insertion
                var.insert(off+1, alt->data()+1);
                _newGt[distance(ents.begin(), e)].push_back(addAllele(var));
            }
        }
    }

    _mergedAlt.resize(_alleleMap.size());
    for (auto i = _alleleMap.begin(); i != _alleleMap.end(); ++i)
        _mergedAlt[i->second] = i->first;

    _merged = _mergedAlt.size() < inputAlts;
}

END_NAMESPACE(Vcf)
