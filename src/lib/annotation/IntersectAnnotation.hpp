#pragma once

#include <boost/function.hpp>
#include <deque>
#include <iostream>

class Bed;
class TranscriptStructure;
class Variant;

class IntersectAnnotation {
public:
    typedef boost::function<void(const Variant&, const TranscriptStructure&)> HitFunc;
    enum Compare {
        BEFORE,
        INTERSECT,
        AFTER
    };

    static Compare cmp(const Variant& a, const TranscriptStructure& b);

    IntersectAnnotation(std::istream& s, const HitFunc& hitFunc, bool firstOnly);

    bool eof() const;
    bool intersect(const Bed& bed);

    bool nextStructure(TranscriptStructure& ts);

protected:
    bool doIntersect(const Variant& v);

protected:
    std::istream& _s;
    HitFunc _hitFunc;
    bool _firstOnly;
    std::deque<TranscriptStructure> _cache;
};
