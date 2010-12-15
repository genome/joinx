#include "SnvIntersector.hpp"

#include "Bed.hpp"
#include "IResultCollector.hpp"
#include "BedStream.hpp"

SnvIntersector::SnvIntersector(BedStream& a, BedStream& b, IResultCollector& rc)
    : _a(a)
    , _b(b)
    , _rc(rc)
{
}

void SnvIntersector::exec() {
    Bed snvA;
    Bed snvB;
    _a.next(snvA);
    _b.next(snvB);

    while (!_a.eof() && !_b.eof()) {
        // TODO: burn off repeats

        int c = snvA.cmp(snvB);
        if (c < 0) {
            _rc.miss(snvA, snvB);
            if (!_a.next(snvA)) break;
        } else if (c > 0) {
            if (!_b.next(snvB)) break;
        } else {
                _rc.hit(snvA, snvB);
            if (!_a.next(snvA)) break;

            // NOTE: do not uncomment this. we do not advance B here because
            // we want to allow for the possibility of repetitions in A.
            // if we advanced B, these would be seen as misses. B will advance
            // naturally once A has passed it
            //
            // if (!_b.next(snvB)) break;
        }
    }

    while (!_a.eof()) {
        _a.next(snvA);
        _rc.miss(snvA, snvB);
    }

    while (!_b.eof()) {
        _b.next(snvB);
    }
}
