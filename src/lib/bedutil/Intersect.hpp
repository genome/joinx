#pragma once

#include <cstring>
#include <deque>

template<typename StreamTypeA, typename StreamTypeB, typename CollectorType>
class Intersect {
public:
    typedef typename StreamTypeA::ValueType TypeA;
    typedef typename StreamTypeB::ValueType TypeB;

    enum Compare {
        BEFORE,
        INTERSECT,
        AFTER
    };

    Intersect(StreamTypeA& a, StreamTypeB& b, CollectorType& rc)
        : _a(a) , _b(b), _rc(rc)
    {}
    virtual ~Intersect() {}

    Compare compare(const TypeA& a, const TypeB& b) {
        int rv = strverscmp(a.chrom().c_str(), b.chrom().c_str());
        if (rv < 0)
            return BEFORE;
        if (rv > 0)
            return AFTER;

        // to handle insertions!
        if (a.start() == a.stop() && b.start() == b.stop() && a.start() == b.start())
            return INTERSECT;

        if (a.stop() <= b.start())
            return BEFORE;
        if (b.stop() <= a.start())
            return AFTER;

        return INTERSECT;
    }

    bool eof() const {
        return _a.eof() || _b.eof();
    }

    bool checkCache(const TypeA& valueA) {
        for (CacheIterator iter = _cache.begin(); iter != _cache.end(); ++iter) {
            Compare cmp = compare(valueA, iter->value);
            if (cmp == BEFORE)
                return false;

            else if (cmp == AFTER) {
                iter = _cache.erase(iter);
                continue;
            }

            bool rv = _rc.hit(valueA, iter->value);
            iter->hit |= rv;
            return rv;
        }
        return false;
    }

    void execute() {
        TypeA valueA;
        while (!_a.eof() && _a.next(valueA)) {
            // burn entries from the cache
            while (!_cache.empty() && compare(valueA, _cache.front().value) == AFTER)
                popCache();

            // look ahead and burn entries from the input file
            TypeB valueB;
            TypeB* peek = NULL;
            if (_cache.empty()) {
                while (!_b.eof() && _b.peek(&peek) && compare(valueA, *peek) == AFTER) {
                    _b.next(valueB);
                    _rc.missB(valueB);
                }
            }

            bool hitA = checkCache(valueA);
            while (!_b.eof() && _b.next(valueB)) {
                Compare cmp = compare(valueA, valueB);
                if (cmp == BEFORE) {
                    cache(valueB, false);
                    break;
                } else if (cmp == AFTER) {
                    _rc.missB(valueB);
                    continue;
                }

                bool rv = _rc.hit(valueA, valueB);
                hitA |= rv;
                cache(valueB, rv);
            }

            if (!hitA) {
                _rc.missA(valueA);
            }
        }
        while (!_cache.empty())
            popCache();
    }

    void cache(const TypeB& valueB, bool hit) {
        _cache.push_back(CacheEntry(valueB, hit));
    }

    void popCache() {
        const CacheEntry& ce = _cache.front();
        if (!ce.hit)
            _rc.missB(ce.value);
        _cache.pop_front();
    }
    
protected:
    struct CacheEntry {
        CacheEntry(const TypeB& v) : value(v), hit(false) {}
        CacheEntry(const TypeB& v, bool hit) : value(v), hit(hit) {}

        TypeB value;
        bool hit;
    };

    typedef typename std::deque<CacheEntry> CacheType;
    typedef typename CacheType::iterator CacheIterator;

    StreamTypeA& _a;
    StreamTypeB& _b;
    CollectorType& _rc;
    CacheType _cache;
};
