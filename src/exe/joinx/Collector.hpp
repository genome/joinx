#pragma once

#include "common/Variant.hpp"
#include "fileformats/Bed.hpp"

#include <iostream>

class Collector {
public:
    Collector(
            bool outputBoth,
            bool exactPos,
            bool exactAllele,
            std::ostream& s,
            std::ostream* missA = NULL,
            std::ostream* missB = NULL
            )
        : _outputBoth(outputBoth)
        , _exactPos(exactPos)
        , _exactAllele(exactAllele)
        , _s(s)
        , _missA(missA)
        , _missB(missB)
        , _hitCount(0)
    {}

    void missA(const Bed& a) {
        if (_missA)
            *_missA << a << "\n";
    }

    void missB(const Bed& b) {
        if (_missB)
            *_missB << b << "\n";
    }

    bool hit(const Bed& a, const Bed& b) {
        Variant va(a);
        Variant vb(b);

        // If we are only outputting A or have set 'unique', then skip 
        // things that we just printed.
        // The core intersector returns the full join of A and B.
        // If we are only outputing A, this can look confusing as
        // each time A intersects something in B, an identical line
        // will be printed.
        if (_hitCount > 0 && !_outputBoth && _lastA.positionMatch(va) && _lastA.alleleMatch(va))
                return true; // already hit
        ++_hitCount;

        if ((_exactPos && !va.positionMatch(vb)) || 
            (_exactAllele && !va.alleleMatch(vb)))
            return false; // reject!

        _lastA = a;
        _s << a;
        if (_outputBoth)
            _s << "\t" << b;
        _s << "\n";

        return true;
    }

protected:
    Variant _lastA;
    bool _outputBoth;
    bool _exactPos;
    bool _exactAllele;
    bool _unique;
    std::ostream& _s;
    std::ostream* _missA;
    std::ostream* _missB;
    uint32_t _hitCount;
};
