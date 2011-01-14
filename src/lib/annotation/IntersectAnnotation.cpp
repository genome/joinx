#include "IntersectAnnotation.hpp"

#include "Iub.hpp"
#include "fileformats/TranscriptStructure.hpp"
#include "common/Variant.hpp"
#include "fileformats/Bed.hpp"

#include <cstring>

using namespace std;

IntersectAnnotation::Compare IntersectAnnotation::cmp(const Variant& a, const TranscriptStructure& b) {
    int rv = strverscmp(a.chrom().c_str(), b.chrom().c_str());
    if (rv < 0)
        return BEFORE;
    if (rv > 0)
        return AFTER;

    if (a.stop() < b.region().start())
        return BEFORE;
    if (b.region().stop() < a.start())
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
    Variant v(bed);

    if (v.start() == v.stop() && v.variant().data().size() == 1 && v.variant().data() != "-") {
        string iub = translateIub(v.variant().data());
        for (string::size_type i = 0; i < iub.size(); ++i) {
            if (v.reference().data()[0] == iub[i])
                continue;

            Variant newVariant(v);
            newVariant.setVariantSequence(Sequence(iub[i]));
            if (!doIntersect(newVariant))
                break;
        }
        return !eof();
    }
    return doIntersect(v);
}

bool IntersectAnnotation::doIntersect(const Variant& v) {
    TranscriptStructure structure;

    // burn off beds from the cache
    while (!_cache.empty() && cmp(v, _cache.front()) == AFTER)
        _cache.pop_front();

    if (_cache.empty()) {
        while (!_s.eof() && nextStructure(structure)) {
            if (cmp(v, structure) != AFTER) {
                _cache.push_back(structure);
                break;
            }
        }
    }

    for (deque<TranscriptStructure>::iterator iter = _cache.begin(); iter != _cache.end();) {
        Compare c = cmp(v, *iter);
        if (c == BEFORE)
            return !eof();
        else if (c == AFTER) {
            iter = _cache.erase(iter);
            continue;
        }

        _hitFunc(v, *iter);
        if (_firstOnly)
            iter = _cache.erase(iter);
        else
            ++iter;
    }

    while (!_s.eof() && nextStructure(structure)) {
        Compare c = cmp(v, structure);
        if (c == BEFORE) {
            _cache.push_back(structure);
            return !eof();
        } else if (c == AFTER) {
            continue;
        }

        _hitFunc(v, structure);
        if (!_firstOnly)
            _cache.push_back(structure);
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
