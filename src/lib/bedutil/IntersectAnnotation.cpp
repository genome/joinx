#include "IntersectAnnotation.hpp"

#include "Bed.hpp"
#include "TranscriptStructure.hpp"

#include <cstring>

using namespace std;

IntersectAnnotation::Compare IntersectAnnotation::cmp(const Bed& a, const TranscriptStructure& b) {
    int rv = strverscmp(a.chrom.c_str(), b.chrom().c_str());
    if (rv < 0)
        return BEFORE;
    if (rv > 0)
        return AFTER;

    if (a.end <= b.start())
        return BEFORE;
    if (b.end() <= a.start)
        return AFTER;

    return INTERSECT;
}

IntersectAnnotation::IntersectAnnotation(istream& s, const HitFunc& hitFunc, bool firstOnly)
    : _s(s)
    , _hitFunc(hitFunc)
    , _firstOnly(firstOnly)
{
}

bool IntersectAnnotation::eof() const {
    return _s.eof() && _cache.empty();
}

bool IntersectAnnotation::intersect(const Bed& bed) {
    TranscriptStructure next;

    // burn off beds from the cache
    while (!_cache.empty() && cmp(bed, _cache.front()) == AFTER)
        _cache.pop_front();

    if (_cache.empty()) {
        while (!_s.eof() && nextStructure(next)) {
            if (cmp(bed, next) != AFTER) {
                _cache.push_back(next);
                break;
            }
        }
    }

    for (deque<TranscriptStructure>::iterator iter = _cache.begin(); iter != _cache.end();) {
        Compare c = cmp(bed, *iter);
        if (c == BEFORE)
            return !eof();
        else if (c == AFTER) {
            iter = _cache.erase(iter);
            continue;
        }

        _hitFunc(bed, *iter);
        if (_firstOnly)
            iter = _cache.erase(iter);
        else
            ++iter;
    }

    while (!_s.eof() && nextStructure(next)) {
        Compare c = cmp(bed, next);
        if (c == BEFORE) {
            _cache.push_back(next);
            return !eof();
        } else if (c == AFTER) {
            continue;
        }

        _hitFunc(bed, next);
        if (!_firstOnly)
            _cache.push_back(next);
    }

    return !eof();
}

bool IntersectAnnotation::nextStructure(TranscriptStructure& ts) {
    if (_s.eof())
        return false;

    string line;
    getline(_s, line);
    if (line.empty())
        return false;

    TranscriptStructure::parseLine(line, ts);
    return true;
}
