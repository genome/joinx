#pragma once

// Utilities for comparing things / establishing orderings.

#include <boost/utility/declval.hpp>

///////////////////////////////////////////////////////////////////////////
// Tags for use with enable_if

// Compare objects return an int <0, =0, or >0 to establish an ordering
// (cf. strcmp)
struct CompareBase {};
// ComparePred objects are predicates (e.g., less than)
// (cf. std::less)
struct ComparePredBase {};

template<typename Op>
struct DerefBinaryOp {
    Op op;

    explicit DerefBinaryOp(Op op = Op())
        : op(op)
    {}

    template<typename ValueType>
    auto operator()(ValueType const& x, ValueType const& y) -> decltype(op(*x, *y)) {
        return op(*x, *y);
    }
};

// Given a "Compare" function, convert it to a less than predicate.
template<
          typename T
        , typename = typename std::enable_if<std::is_base_of<CompareBase, T>::value>::type
        >
struct CompareToLessThan : ComparePredBase {
    CompareToLessThan(T cmp = T())
        : cmp(cmp)
    {}

    template<typename U>
    bool operator()(U const& x, U const& y) const {
        return cmp(x, y) < 0;
    }

    T cmp;
};

// Given a "Compare" function, convert it to a greater than predicate.
template<
          typename T
        , typename = typename std::enable_if<std::is_base_of<CompareBase, T>::value>::type
        >
struct CompareToGreaterThan : ComparePredBase {
    CompareToGreaterThan(T cmp = T())
        : cmp(cmp)
    {}

    template<typename U>
    bool operator()(U const& x, U const& y) const {
        return cmp(x, y) > 0;
    }

    T cmp;
};

// This is a helper class that defines relational operators
// (>, <, ==, !=, ...) for simple structs that define value_type
// and have a single member: value_type value;
template<typename T>
struct ValueBasedRelOps {
    friend bool operator<(T const& lhs, T const& rhs) {
        return lhs.value < rhs.value;
    }

    friend bool operator>(T const& lhs, T const& rhs) {
        return lhs.value > rhs.value;
    }

    friend bool operator<=(T const& lhs, T const& rhs) {
        return lhs.value <= rhs.value;
    }

    friend bool operator>=(T const& lhs, T const& rhs) {
        return lhs.value >= rhs.value;
    }

    friend bool operator==(T const& lhs, T const& rhs) {
        return lhs.value == rhs.value;
    }

    friend bool operator!=(T const& lhs, T const& rhs) {
        return lhs.value != rhs.value;
    }
};
