#pragma once

#include <boost/format.hpp>
#include <cstring>
#include <list>
#include <stdexcept>

template<typename StreamTypeA, typename StreamTypeB, typename CollectorType>
class Intersect {
public: // types and data
    // value types
    typedef typename StreamTypeA::ValueType TypeA;
    typedef typename StreamTypeB::ValueType TypeB;

    // cache types
    struct CacheEntry {
        CacheEntry(const TypeB& v) : value(v), hit(false) {}
        CacheEntry(const TypeB& v, bool hit) : value(v), hit(hit) {}

        TypeB value;
        bool hit;
    };

    typedef typename std::list<CacheEntry> CacheType;
    typedef typename CacheType::iterator CacheIterator;

    // compare results
    enum Compare {
        BEFORE,
        INTERSECT,
        AFTER
    };


public: // code
    Intersect(StreamTypeA& a, StreamTypeB& b, CollectorType& rc, bool adjacentInsertions = false)
        : _a(a) , _b(b), _rc(rc)
        , _compareFunc(adjacentInsertions ? compareWithAdjacentInsertions : compare)
    {
    }

    virtual ~Intersect() {}

    static Compare compare(const TypeA& a, const TypeB& b) {
        int rv = strverscmp(a.chrom().c_str(), b.chrom().c_str());
        if (rv < 0)
            return BEFORE;
        if (rv > 0)
            return AFTER;

        // to handle identical insertions
        if (a.start() == a.stop() && b.start() == b.stop() && a.start() == b.start())
            return INTERSECT;

        if (a.stop() <= b.start())
            return BEFORE;
        if (b.stop() <= a.start())
            return AFTER;

        return INTERSECT;
    }

    static Compare compareWithAdjacentInsertions(const TypeA& a, const TypeB& b) {
        int rv = strverscmp(a.chrom().c_str(), b.chrom().c_str());
        if (rv < 0)
            return BEFORE;
        if (rv > 0)
            return AFTER;

        // to handle adjacent/exact match insertions!
        if ((a.length() == 0 && (a.start() == b.start() || a.start() == b.stop())) ||
            (b.length() == 0 && (b.start() == a.start() || b.start() == a.stop())) )
        {
            return INTERSECT;
        }

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
        bool rv = false;
        auto iter = _cache.begin();
        while (iter != _cache.end()) {
            Compare cmp = _compareFunc(valueA, iter->value);
            if (cmp == BEFORE) {
                break;
            } else if (cmp == AFTER) {
                auto toRemove = iter;
                ++iter;
                cacheRemove(toRemove);
                continue;
            }

            bool isHit = _rc.hit(valueA, iter->value);
            rv |= isHit;
            iter->hit |= isHit;
            ++iter;
        }
        return rv;
    }

    template<typename T, typename S>
    bool advanceSorted(S& stream, T& value) {
        using std::runtime_error;
        using boost::format;
        T* peek = NULL;
        if (stream.peek(&peek) && _compareFunc(*peek, value) == BEFORE)
            throw runtime_error(str(format("Unsorted data found in stream %1%\n'%2%' follows '%3%'") %stream.name() %peek->toString() %value.toString()));
        return stream.next(value);
    }

    void execute() {
        TypeA valueA;
        TypeB valueB;
        while (!_a.eof() && advanceSorted(_a, valueA)) {
            // burn entries from the cache
            while (!_cache.empty() && _compareFunc(valueA, _cache.front().value) == AFTER)
                popCache();

            // look ahead and burn entries from the input file
            TypeB* peek = NULL;
            if (_cache.empty()) {
                while (!_b.eof() && _b.peek(&peek) && _compareFunc(valueA, *peek) == AFTER) {
                    advanceSorted(_b, valueB);
                    if (_rc.wantMissB())
                        _rc.missB(valueB);
                }
            }

            bool hitA = checkCache(valueA);
            while (!_b.eof() && advanceSorted(_b, valueB)) {
                Compare cmp = _compareFunc(valueA, valueB);
                if (cmp == BEFORE) {
                    cache(valueB, false);
                    break;
                } else if (cmp == AFTER) {
                    if (_rc.wantMissB())
                        _rc.missB(valueB);
                    continue;
                }

                bool rv = _rc.hit(valueA, valueB);
                hitA |= rv;
                cache(valueB, rv);
            }

            if (!hitA && _rc.wantMissA()) {
                _rc.missA(valueA);
            }
        }
        while (_rc.wantMissB() && !_cache.empty())
            popCache();
        while (_rc.wantMissB() && !_b.eof() && advanceSorted(_b, valueB))
            _rc.missB(valueB);
    }

    void cache(const TypeB& valueB, bool hit) {
        _cache.push_back(CacheEntry(valueB, hit));
    }

    void cacheRemove(const CacheIterator& iter) {
        if (!iter->hit && _rc.wantMissB())
            _rc.missB(iter->value);
        _cache.erase(iter);
    }

    void popCache() {
        cacheRemove(_cache.begin());
    }

protected:
    StreamTypeA& _a;
    StreamTypeB& _b;
    CollectorType& _rc;
    CacheType _cache;
    Compare (*_compareFunc)(const TypeA&, const TypeB&);
};
