#pragma once

#include <boost/function.hpp>
#include <deque>
#include <iostream>

class Bed;
class TranscriptStructure;

class IntersectAnnotation {
public:
    typedef boost::function<void(const Bed&, const TranscriptStructure&)> HitFunc;
    enum Compare {
        BEFORE,
        INTERSECT,
        AFTER
    };

    static Compare cmp(const Bed& a, const TranscriptStructure& b);

    IntersectAnnotation(std::istream& s, const HitFunc& hitFunc, bool firstOnly);

    bool eof() const;
    bool intersect(const Bed& bed);

    bool nextStructure(TranscriptStructure& ts);

protected:
    std::istream& _s;
    HitFunc _hitFunc;
    bool _firstOnly;
    std::deque<TranscriptStructure> _cache;
};
