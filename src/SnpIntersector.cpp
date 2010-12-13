#include "SnpIntersector.hpp"

#include "Bed.hpp"
#include "IResultCollector.hpp"
#include "SnpStream.hpp"

SnpIntersector::SnpIntersector(SnpStream& a, SnpStream& b, IResultCollector& rc)
    : _a(a)
    , _b(b)
    , _rc(rc)
{
}

void SnpIntersector::exec() {
    Bed snpA;
    Bed snpB;
    _a.nextSnp(snpA);
    _b.nextSnp(snpB);

    while (!_a.eof() && !_b.eof()) {
        // TODO: burn off repeats

        int c = snpA.cmp(snpB);
        if (c < 0) {
            _rc.miss(snpA, snpB);
            if (!_a.nextSnp(snpA)) break;
        } else if (c > 0) {
            if (!_b.nextSnp(snpB)) break;
        } else {
                _rc.hit(snpA, snpB);
            if (!_a.nextSnp(snpA)) break;

            // NOTE: do not uncomment this. we do not advance B here because
            // we want to allow for the possibility of repetitions in A.
            // if we advanced B, these would be seen as misses. B will advance
            // naturally once A has passed it
            //
            // if (!_b.nextSnp(snpB)) break;
        }
    }

    while (!_a.eof()) {
        _a.nextSnp(snpA);
        _rc.miss(snpA, snpB);
    }

    while (!_b.eof()) {
        _b.nextSnp(snpB);
    }
}
