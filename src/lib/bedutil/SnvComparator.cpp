#include "SnvComparator.hpp"

#include "fileformats/Bed.hpp"
#include "IResultCollector.hpp"
#include "fileformats/BedStream.hpp"

SnvComparator::SnvComparator(BedStream& a, BedStream& b, IResultCollector& rc)
    : _a(a)
    , _b(b)
    , _rc(rc)
{
}

void SnvComparator::exec() {
    Bed snvA;
    Bed snvB;
    Bed *peek = NULL;
    _a >> snvA;
    _b >> snvB;

    while (!_a.eof() && !_b.eof()) {
        // TODO: burn off repeats

        int c = snvA.cmp(snvB);
        if (c < 0) {
            _rc.missA(snvA);
            while (_a.peek(&peek) && peek->cmp(snvA) == 0) {
                _a >> snvA;
                _rc.missA(snvA);
            }
            _a >> snvA;
        } else if (c > 0) {
            _rc.missB(snvB);
            while (_b.peek(&peek) && peek->cmp(snvB) == 0) {
                _b >> snvB;
                _rc.missB(snvB);
            }
            _b >> snvB;
        } else {
            _rc.hitA(snvA);
            _rc.hitB(snvB);
            while (_a.peek(&peek) && peek->cmp(snvA) == 0) {
                _a >> snvA;
                _rc.hitA(snvA);
            }

            while (_b.peek(&peek) && peek->cmp(snvB) == 0) {
                _b >> snvB;
                _rc.hitB(snvB);
            }
            if (!_a.eof()) _a >> snvA;
            if (!_b.eof()) _b >> snvB;
        }
    }

    while (!_a.eof()) {
        _a >> snvA;
        _rc.missA(snvA);
    }

    while (!_b.eof()) {
        _b >> snvB;
        _rc.missB(snvB);
    }
}
