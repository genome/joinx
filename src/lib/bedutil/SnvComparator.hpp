#pragma once

template<typename ReaderType, typename CollectorType>
class SnvComparator {
public: // nested class: interface for result collection
    typedef typename ReaderType::ValueType ValueType;

    SnvComparator(ReaderType& a, ReaderType& b, CollectorType& rc);

    void exec();

protected:
    ReaderType& _a;
    ReaderType& _b;
    CollectorType& _rc;
};

template<typename ReaderType, typename CollectorType>
inline SnvComparator<ReaderType,CollectorType>::SnvComparator(ReaderType& a, ReaderType& b, CollectorType& rc)
    : _a(a)
    , _b(b)
    , _rc(rc)
{
}

template<typename ReaderType, typename CollectorType>
inline void SnvComparator<ReaderType,CollectorType>::exec() {
    ValueType snvA;
    ValueType snvB;
    ValueType *peek(0);
    _a.next(snvA);
    _b.next(snvB);

    while (!_a.eof() && !_b.eof()) {
        // TODO: burn off repeats

        int c = snvA.cmp(snvB);
        if (c < 0) {
            _rc.missA(snvA);
            while (_a.peek(&peek) && peek->cmp(snvA) == 0) {
                _a.next(snvA);
                _rc.missA(snvA);
            }
            _a.next(snvA);
        } else if (c > 0) {
            _rc.missB(snvB);
            while (_b.peek(&peek) && peek->cmp(snvB) == 0) {
                _b.next(snvB);
                _rc.missB(snvB);
            }
            _b.next(snvB);
        } else {
            _rc.hitA(snvA);
            _rc.hitB(snvB);
            while (_a.peek(&peek) && peek->cmp(snvA) == 0) {
                _a.next(snvA);
                _rc.hitA(snvA);
            }

            while (_b.peek(&peek) && peek->cmp(snvB) == 0) {
                _b.next(snvB);
                _rc.hitB(snvB);
            }
            if (!_a.eof()) _a.next(snvA);
            if (!_b.eof()) _b.next(snvB);
        }
    }

    while (!_a.eof()) {
        _a.next(snvA);
        _rc.missA(snvA);
    }

    while (!_b.eof()) {
        _b.next(snvB);
        _rc.missB(snvB);
    }
}

