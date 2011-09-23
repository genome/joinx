#pragma once

#include "bedutil/IntersectionOutputFormatter.hpp"
#include "fileformats/Variant.hpp"
#include "fileformats/Bed.hpp"

#include <iostream>

class Collector {
public:
    Collector(
            bool outputBoth,
            bool exactPos,
            bool exactAllele,
            bool iubMatch,
            bool dbsnpMatch,
            IntersectionOutput::Formatter& outputFormatter,
            std::ostream* missA = NULL,
            std::ostream* missB = NULL
            )
        : _outputBoth(outputBoth)
        , _exactPos(exactPos)
        , _exactAllele(exactAllele)
        , _iubMatch(iubMatch)
        , _dbsnpMatch(dbsnpMatch)
        , _outputFormatter(outputFormatter)
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

    bool wantMissA() const {
        return _missA;
    }

    bool wantMissB() const {
        return _missB;
    }

    bool hit(const Bed& a, const Bed& b) {
        Variant va(a);
        Variant vb(b);

        // If we are only outputting A then skip things that we just printed.
        // The core intersector returns the full join of A and B.
        // If we are only outputing A, this can look confusing as
        // each time A intersects something in B, an identical line
        // will be printed.
        if (_hitCount > 0 && !_outputBoth &&
            _lastA.positionMatch(va) && _lastA.alleleMatch(va))
        {
                return true; // already hit
        }
        ++_hitCount;

        // TODO: clean this up! stop using bools and pass in a mode or functor
        // i.e., flatten this
        if (_exactAllele) {
            if (!va.positionMatch(vb))
                return false;

            if (_dbsnpMatch) {
                if (!va.alleleDbSnpMatch(vb))
                    return false;
            } else if (_iubMatch) {
                if (!va.allelePartialMatch(vb))
                    return false;
            } else if (!va.alleleMatch(vb)) {
                return false;
            }

        } else if (_exactPos && !va.positionMatch(vb))
            return false;

        _lastA = va;
        _outputFormatter.output(a, b);

        return true;
    }

protected:
    Variant _lastA;
    bool _outputBoth;
    bool _exactPos;
    bool _exactAllele;
    bool _iubMatch;
    bool _dbsnpMatch;
    IntersectionOutput::Formatter& _outputFormatter;
    std::ostream* _missA;
    std::ostream* _missB;
    uint32_t _hitCount;
};
