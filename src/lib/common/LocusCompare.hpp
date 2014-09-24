#pragma once

#include "CoordinateView.hpp"

#include <boost/tti/has_type.hpp>
#include <boost/utility/declval.hpp>

#include <cstring>
#include <type_traits>

#include <iostream>
#include <typeinfo>

namespace {
    BOOST_TTI_HAS_TYPE(DefaultCompare)

    struct StartAndStop;
    struct StartOnly;
}

template<typename Compare>
struct DerefCompare {
    explicit DerefCompare(Compare cmp = Compare())
        : cmp(cmp)
    {}

    // FIXME: gcc4.4 can't figure out
    // decltype(boost::declval<Compare>()(x, y) for the return value which
    // might not be bool in general
    template<typename ValueType>
    bool operator()(ValueType const& x, ValueType const& y)
    {
        return cmp(*x, *y);
    }

    Compare cmp;
};


template<
          typename CoordView = DefaultCoordinateView
        , typename Method = StartAndStop
        , typename Enable = void
        >
struct LocusCompare;

template<typename CoordView>
struct LocusCompare<
          CoordView
        , StartOnly
        , typename std::enable_if<
            std::is_base_of<CoordinateViewBaseTag, CoordView>::value
            >::type
        >
{
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

        return 0;
    }

    template<typename ValueType>
    typename std::enable_if<std::is_pointer<ValueType>::value, int>::type
    operator()(ValueType const& x, ValueType const& y) const {
        return (*this)(*x, *y);
    }

private:
    CoordView cv;
};

template<typename CoordView>
struct LocusCompare<
          CoordView
        , StartAndStop
        , typename std::enable_if<
            std::is_base_of<CoordinateViewBaseTag, CoordView>::value
            >::type
        >
{
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

    template<typename ValueType>
    typename std::enable_if<std::is_pointer<ValueType>::value, int>::type
    operator()(ValueType const& x, ValueType const& y) const {
        return (*this)(*x, *y);
    }

private:
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

template<typename T>
struct CompareToGreaterThan {
    CompareToGreaterThan(T cmp = T())
        : cmp(cmp)
    {}

    template<typename U>
    bool operator()(U const& x, U const& y) const {
        return cmp(x, y) > 0;
    }

    T cmp;
};

template<typename T>
typename std::enable_if<
          has_type_DefaultCompare<T>::value
        , bool
        >::type
operator<(T const& x, T const& y) {
    return CompareToLessThan<typename T::DefaultCompare>()(x, y);
}

template<typename T>
typename std::enable_if<
          has_type_DefaultCompare<T>::value
        , bool
        >::type
operator>(T const& x, T const& y) {
    return CompareToGreaterThan<typename T::DefaultCompare>()(x, y);
}
