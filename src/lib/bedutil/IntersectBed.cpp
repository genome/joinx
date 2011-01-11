#include "IntersectBed.hpp"

#include "Bed.hpp"
#include "BedStream.hpp"

#include <cstring>

using namespace std;

IntersectBed::BedCompare IntersectBed::cmpBeds(const Bed& a, const Bed& b) {
    int rv = strverscmp(a.chrom.c_str(), b.chrom.c_str());
    if (rv < 0)
        return BEFORE;
    if (rv > 0)
        return AFTER;

    if (a.end <= b.start)
        return BEFORE;
    if (b.end <= a.start)
        return AFTER;

    return INTERSECT;
}

IntersectBed::IntersectBed(BedStream& s, const HitFunc& hitFunc, bool firstOnly)
    : _s(s)
    , _hitFunc(hitFunc)
    , _firstOnly(firstOnly)
{
}

bool IntersectBed::eof() const {
    return _s.eof() && _cache.empty();
}

bool IntersectBed::intersect(const Bed& bed) {
    Bed next;

    // burn off beds from the cache
    while (!_cache.empty() && cmpBeds(bed, _cache.front()) == AFTER)
        _cache.pop_front();

    Bed* peek = NULL;
    if (_cache.empty())
        while (!_s.eof() && _s.peek(&peek) && cmpBeds(bed, *peek) == AFTER)
            _s.next(next);

    for (deque<Bed>::iterator iter = _cache.begin(); iter != _cache.end();) {
        BedCompare cmp = cmpBeds(bed, *iter);
        if (cmp == BEFORE)
            return !eof();
        else if (cmp == AFTER) {
            iter = _cache.erase(iter);
            continue;
        }

        _hitFunc(bed, *iter);
        if (_firstOnly)
            iter = _cache.erase(iter);
        else
            ++iter;
    }

    while (!_s.eof() && _s.next(next)) {
        BedCompare cmp = cmpBeds(bed, next);
        if (cmp == BEFORE) {
            _cache.push_back(next);
            return !eof();
        } else if (cmp == AFTER) {
            continue;
        }


        _hitFunc(bed, next);
        if (!_firstOnly)
            _cache.push_back(next);
    }

    return !eof();
}
