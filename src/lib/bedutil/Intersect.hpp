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

    void execute() {
        while (!_a.eof()) {
            TypeA valueA;
            _a.next(valueA);

            // burn entries from the cache
            while (!_cache.empty() && compare(valueA, _cache.front()) == AFTER)
                _cache.pop_front();

            // look ahead and burn entries from the input file
            TypeB valueB;
            TypeB* peek = NULL;
            if (_cache.empty())
                while (!_b.eof() && _b.peek(&peek) && compare(valueA, *peek) == AFTER)
                    _b.next(valueB);

            for (CacheIterator iter = _cache.begin(); iter != _cache.end(); ++iter) {
                Compare cmp = compare(valueA, *iter);
                if (cmp == BEFORE)
                    break;

                else if (cmp == AFTER) {
                    iter = _cache.erase(iter);
                    continue;
                }

                _rc.hit(valueA, *iter);
            }

            while (!_b.eof() && _b.next(valueB)) {
                Compare cmp = compare(valueA, valueB);
                if (cmp == BEFORE) {
                    _cache.push_back(valueB);
                    break;
                } else if (cmp == AFTER) {
                    continue;
                }

                _rc.hit(valueA, valueB);
                _cache.push_back(valueB);
            }
        }
    }
    
protected:
    typedef typename std::deque<TypeB> CacheType;
    typedef typename std::deque<TypeB>::iterator CacheIterator;

    StreamTypeA& _a;
    StreamTypeB& _b;
    CollectorType& _rc;
    std::deque<TypeB> _cache;
};
