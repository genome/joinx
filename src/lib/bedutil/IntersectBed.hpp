#pragma once

#include <boost/function.hpp>
#include <deque>

class Bed;
class BedStream; 

class IntersectBed {
public:
    typedef boost::function<void(const Bed&, const Bed&)> HitFunc;
    enum BedCompare {
        BEFORE,
        INTERSECT,
        AFTER
    };

    static BedCompare cmpBeds(const Bed& a, const Bed& b);

    IntersectBed(BedStream& s, const HitFunc& hitFunc, bool firstOnly);

    bool eof() const;
    bool intersect(const Bed& bed);

protected:
    BedStream& _s;
    HitFunc _hitFunc;
    bool _firstOnly;
    std::deque<Bed> _cache;
};
