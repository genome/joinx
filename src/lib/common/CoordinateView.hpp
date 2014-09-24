#pragma once

#include <string>

struct CoordinateViewBaseTag {};

struct DefaultCoordinateView : CoordinateViewBaseTag {
    template<typename T>
    auto chrom(T const& x) const -> decltype(x.chrom()) {
        return x.chrom();
    }

    template<typename T>
    auto start(T const& x) const -> decltype(x.start()) {
        return x.start();
    }

    template<typename T>
    auto stop(T const& x) const -> decltype(x.stop()) {
        return x.stop();
    }
};

struct UnpaddedCoordinateView : CoordinateViewBaseTag {
    template<typename T>
    auto chrom(T const& x) const -> decltype(x.chrom()) {
        return x.chrom();
    }

    template<typename T>
    auto start(T const& x) const -> decltype(x.startWithoutPadding()) {
        return x.startWithoutPadding();
    }

    template<typename T>
    auto stop(T const& x) const -> decltype(x.stopWithoutPadding()) {
        return x.stopWithoutPadding();
    }
};
