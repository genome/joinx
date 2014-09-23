#pragma once

#include "CoordinateView.hpp"

#include <cstring>

template<typename CoordView = DefaultCoordinateView>
struct LocusCompare {
    explicit LocusCompare(CoordView cv = CoordView())
        : cv(cv)
    {}

    template<typename ValueType>
    int operator()(ValueType const& x, ValueType const& y) const {
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

    CoordView cv;
};

