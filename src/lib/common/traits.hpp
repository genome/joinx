#pragma once

#include "namespaces.hpp"

#include <memory>
#include <type_traits>

// these are just for static_assert tests
#include <iterator>
#include <vector>

BEGIN_NAMESPACE(traits)

template<typename T>
struct is_unique_ptr : std::false_type {};

template<typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

static_assert(!is_unique_ptr<int>::value, "is not a unique ptr");
static_assert(is_unique_ptr<std::unique_ptr<int>>::value, "is a unique ptr");
static_assert(
      is_unique_ptr<
        typename std::iterator_traits<std::vector<std::unique_ptr<int>>::iterator>::value_type
        >::value
    , "is a unique ptr");

END_NAMESPACE(traits)
