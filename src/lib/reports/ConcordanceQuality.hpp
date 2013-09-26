#pragma once

#include "fileformats/Bed.hpp"
#include "common/cstdint.hpp"

#include <cstdlib>
#include <map>
#include <ostream>
#include <sstream>
#include <utility>

class ConcordanceQuality {
public:
    ConcordanceQuality() : _hitCount(0), _missCount(0) {}

    unsigned qualityLevel(const Bed& snv);

    void hitA(const Bed& snv) {
        using namespace std;
        unsigned qual = qualityLevel(snv);
        ++_hitCount;
        pair<MapType::iterator,bool> p = _hitMap.insert(make_pair(qual, 1));
        if (!p.second)
            ++p.first->second;
    }

    void hitB(const Bed& snv) {
    }

    // don't care about misses in B
    void missB(const Bed& b) {
    }

    void missA(const Bed& snv) {
        using namespace std;
        ++_missCount;
        unsigned qual = qualityLevel(snv);

        pair<MapType::iterator,bool> p = _missMap.insert(make_pair(qual, 1));
        if (!p.second)
            ++p.first->second;
    }

    uint64_t hits() const {
        return _hitCount;
    }

    uint64_t misses() const {
        return _missCount;
    }

    uint64_t missCount(unsigned qual) const {
        MapType::const_iterator iter = _missMap.find(qual);
        if (iter == _missMap.end())
            return 0;
        return iter->second;
    }

    void report(std::ostream& s) {
        using namespace std;
        s << "Results by quality level:" << endl << endl;
        unsigned total = 0;
        unsigned hits = 0;
        typedef MapType::const_reverse_iterator IterType;

        unsigned maxHit = 0;
        unsigned maxMiss = 0;
        if (_hitMap.rbegin() != _hitMap.rend())
            maxHit = _hitMap.rbegin()->first;
        if (_missMap.rbegin() != _missMap.rend())
            maxHit = _missMap.rbegin()->first;
        unsigned maxQuality = max(maxHit, maxMiss);
        for(int i = maxQuality; i >= 0; --i) {
            total += _hitMap[i] + _missMap[i];
            hits += _hitMap[i];
            s << i << ": " << hits << "/" << total << "\t";
            if (total == 0)
                s << "0%" << endl;
            else
                s << 100.0 * hits / float(total) << "%" << endl;
        }
        s << "Overall concordance: " << _hitCount/float(_hitCount+_missCount)*100.0 << endl;
    }

protected:
    uint64_t _hitCount;
    uint64_t _missCount;
    typedef std::map<unsigned, uint64_t> MapType;
    MapType _hitMap;
    MapType _missMap;
};
