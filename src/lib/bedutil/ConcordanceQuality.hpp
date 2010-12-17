#pragma once

#include "IResultCollector.hpp"
#include "Bed.hpp"
#include "intconfig.hpp"

#include <cstdlib>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>

class ConcordanceQuality : public IResultCollector {
public:
    ConcordanceQuality() : _hitCount(0), _missCount(0) {}

    unsigned qualityLevel(const Bed& snv) {
        using namespace std;

        char* end = NULL;
        unsigned qual = strtoul(snv.qual.c_str(), &end, 10);
        if (end != &snv.qual[snv.qual.size()]) {
            stringstream ss;
            ss << "Failed to extract quality value from string '" << snv.qual 
                << "'.";
            throw runtime_error(ss.str());
        }

        return qual;
    }

    void hit(const Bed& snv, const Bed& junk) {
        hitA(snv);
    }

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
        s << "Results by quality level:" << endl;
        unsigned total = 0;
        unsigned hits = 0;
        typedef MapType::const_reverse_iterator IterType;
        for(IterType iter = _hitMap.rbegin(); iter != _hitMap.rend(); ++iter) {
            total += iter->second + missCount(iter->first);
            hits += iter->second;
            s << iter->first << ": " << hits << "/" << total << " ";
            if (total == 0)
                s << "-%" << endl;
            else
                s << 100.0 * hits / float(total) << "%" << endl;
        }

        cout << "Overall concordance: " << _hitCount/float(_hitCount+_missCount)*100.0 << endl;
    }

protected:
    uint64_t _hitCount;
    uint64_t _missCount;
    typedef std::map<unsigned, uint64_t> MapType;
    MapType _hitMap;
    MapType _missMap;
};
