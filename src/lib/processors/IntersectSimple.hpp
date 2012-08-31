#pragma once

#include "common/UnsortedDataError.hpp"

#include <boost/format.hpp>
#include <cstring>
#include <deque>
#include <stdexcept>

template<typename StreamTypeA, typename StreamTypeB, typename OutType>
class IntersectSimple {
public: // types and data
    // value types
    typedef typename StreamTypeA::ValueType TypeA;
    typedef typename StreamTypeB::ValueType TypeB;
    typedef TypeB CacheEntry;

    // compare results
    enum Compare {
        BEFORE,
        INTERSECT,
        AFTER
    };

public:
    IntersectSimple(StreamTypeA& sa, StreamTypeB& sb, OutType& out)
        : _sa(sa)
        , _sb(sb)
        , _out(out)
    {
    }

    template<typename TA, typename TB>
    Compare compare(const TA& a, const TB& b) const {
        int rv = strverscmp(a.chrom().c_str(), b.chrom().c_str());
        if (rv < 0)
            return BEFORE;
        if (rv > 0)
            return AFTER;

        // to handle identical insertions
        if (a.start() == a.stop() 
            && b.start() == b.stop()
            && a.start() == b.start())
        {
            return INTERSECT;
        }

        if (a.stop() <= b.start())
            return BEFORE;
        if (b.stop() <= a.start())
            return AFTER;

        return INTERSECT;
    }

    // Extract the next member from stream into value if the next value does
    // not compare as less than to the existing value (enforcing sorted data)
    template<typename T, typename S>
    bool advanceSorted(S& stream, T& value) {
        using std::runtime_error;
        using boost::format;
        T* peek = NULL;
        if (stream.peek(&peek) && compare(*peek, value) == BEFORE)
            throw UnsortedDataError(str(format(
                "Unsorted data found in stream %1%\n'%2%' follows '%3%'")
                %stream.name() %peek->toString() %value.toString())
                );
        return stream.next(value);
    }

    void setCacheFor(TypeA const& value) {
        // clear all entries from the cache that come completely before 'value'
        while (!_cache.empty() && compare(value, _cache.front()) == AFTER)
            _cache.pop_front();

        while (!_sb.eof() && advanceSorted(_sb, _valueB)) {
            Compare cmp = compare(value, _valueB);
            if (cmp == AFTER)
                continue;
            else if (cmp == INTERSECT)
                _cache.push_back(_valueB);
            else
                break;
        }
    }

    void execute() {
        TypeA valueA;
        while (!_sa.eof() && advanceSorted(_sa, valueA)) {
            // clear all entries from the cache that precede 'valueA'
            while (!_cache.empty() && compare(valueA, _cache.front()) == AFTER)
                _cache.pop_front();

            // capture any hits remaining in the cache
            std::vector<TypeB> hits;
            for (auto iter = _cache.begin(); iter != _cache.end(); ++iter) {
                if (compare(valueA, *iter) == INTERSECT)
                    hits.push_back(*iter);
            }

            while (!_sb.eof() && advanceSorted(_sb, _valueB)) {
                Compare cmp = compare(valueA, _valueB);
                if (cmp == AFTER) {
                    // cmp == AFTER ==> A comes after B, B can never hit
                    // anything further down the stream in A
                    continue;
                }
                _cache.push_back(_valueB);
                if (cmp == INTERSECT)
                    hits.push_back(_valueB);
                else
                    break;
            }

            _out(valueA, hits);
        }
    }

protected:
    StreamTypeA& _sa;
    StreamTypeB& _sb;
    OutType& _out;
    // The cache alone is not enough to keep track of what the last value
    // read from stream B, so we keep a member variable in order to ensure
    // that stream B is in fact sorted.
    TypeB _valueB;
    std::deque<CacheEntry> _cache;
};
