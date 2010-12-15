#include "SnvIntersector.hpp"

#include "Bed.hpp"
#include "IResultCollector.hpp"
#include "SnvStream.hpp"

SnvIntersector::SnvIntersector(SnvStream& a, SnvStream& b, IResultCollector& rc)
    : _a(a)
    , _b(b)
    , _rc(rc)
{
}

void SnvIntersector::exec() {
    Bed snvA;
    Bed snvB;
    _a.nextSnv(snvA);
    _b.nextSnv(snvB);

    while (!_a.eof() && !_b.eof()) {
        // TODO: burn off repeats

        int c = snvA.cmp(snvB);
        if (c < 0) {
            _rc.miss(snvA, snvB);
            if (!_a.nextSnv(snvA)) break;
        } else if (c > 0) {
            if (!_b.nextSnv(snvB)) break;
        } else {
                _rc.hit(snvA, snvB);
            if (!_a.nextSnv(snvA)) break;

            // NOTE: do not uncomment this. we do not advance B here because
            // we want to allow for the possibility of repetitions in A.
            // if we advanced B, these would be seen as misses. B will advance
            // naturally once A has passed it
            //
            // if (!_b.nextSnv(snvB)) break;
        }
    }

    while (!_a.eof()) {
        _a.nextSnv(snvA);
        _rc.miss(snvA, snvB);
    }

    while (!_b.eof()) {
        _b.nextSnv(snvB);
    }
}
