#pragma once

#include <boost/functional/hash.hpp>

#include <cstddef>
#include <tuple>

// TupleHasher< std::tuple<...> > can be used as the hasher for
// boost::unordered_map<std::tuple<...>, X>, e.g.:
//
// typedef std::tuple<int, std::string> MyTuple;
// boost::unordered_map<MyTuple, int, TupleHasher<MyTuple>>

template<typename TupleType, std::size_t idx_ = std::tuple_size<TupleType>::value - 1>
struct TupleHasher {
    std::size_t operator()(TupleType const& x) const {
        std::size_t seed = 0;
        boost::hash_combine(seed, std::get<idx_>(x));
        TupleHasher<TupleType, idx_ - 1> next;
        boost::hash_combine(seed, next(x));
        return seed;
    }
};

template<typename TupleType>
struct TupleHasher<TupleType, 0> {
    std::size_t operator()(TupleType const& x) const {
        return boost::hash_value(std::get<0>(x));
    }
};


