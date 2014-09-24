#pragma once

#include "CoordinateView.hpp"

#include <cstring>

template<typename CoordView = DefaultCoordinateView>
struct LocusCompare {
    explicit LocusCompare(CoordView cv = CoordView())
        : cv(cv)
    {}

    template<typename ValueType>
    typename std::enable_if<!std::is_pointer<ValueType>::value, int>::type
    operator()(ValueType const& x, ValueType const& y) const {
        int chr = strverscmp(cv.chrom(x).c_str(), cv.chrom(y).c_str());
        if (chr != 0)
            return chr;

        if (cv.start(x) < cv.start(y))
            return -1;
        if (cv.start(y) < cv.start(x))
            return 1;

        if (cv.stop(x) < cv.stop(y))
            return -1;
        if (cv.stop(y) < cv.stop(x))
            return 1;

        return 0;
    }

    // This is locus compare; we're not here to compare pointer addresses
    template<typename ValueType>
    typename std::enable_if<std::is_pointer<ValueType>::value, int>::type
    operator()(ValueType x, ValueType y) const {
        return (*this)(*x, *y);
    }

    CoordView cv;
};

// FIXME: move somewhere else

template<typename T>
struct CompareToLessThan {
    CompareToLessThan(T cmp = T())
        : cmp(cmp)
    {}

    template<typename U>
    bool operator()(U const& x, U const& y) const {
        return cmp(x, y) < 0;
    }

    T cmp;
};
