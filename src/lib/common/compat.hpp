#pragma once

#include "traits.hpp"
#include "namespaces.hpp"

#include <boost/type_traits.hpp>

#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

BEGIN_NAMESPACE(compat)

#ifdef CXX11_CANT_SORT_UNIQUE_PTR // old stdlibc++ can't

// we don't need the version without a predicate yet it is omitted

template<typename Iter, typename Compare>
typename std::enable_if<
        traits::is_unique_ptr<typename std::iterator_traits<Iter>::value_type>::value
        >::type
sort(Iter first, Iter last, Compare cmp) {
    auto save = first;
    typedef typename boost::decay<decltype(**first)>::type Item;
    std::size_t sz = std::distance(first, last);
    std::vector<Item*> items(sz);
    for (std::size_t i = 0; i < sz; ++i, ++first) {
        items[i] = first->get();
    }

    std::sort(items.begin(), items.end(), cmp);

    first = save;
    for (std::size_t i = 0; i < sz; ++i, ++first) {
        first->release();
        first->reset(items[i]);
    }
}

template<typename Iter, typename Compare>
typename std::enable_if<
        !traits::is_unique_ptr<typename std::iterator_traits<Iter>::value_type>::value
        >::type
sort(Iter first, Iter last, Compare cmp) {
    std::sort(first, last, cmp);
}

#else

template<typename Iter, typename Compare>
void sort(Iter first, Iter last, Compare cmp) {
    std::sort(first, last, cmp);
}

#endif

END_NAMESPACE(compat)

#ifdef CXX14_NO_MAKE_UNIQUE
BEGIN_NAMESPACE(std) // OH NO!

// We don't handle the array cases; this is all we need for now
template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>{new T(forward<Args>(args)...)};
}

END_NAMESPACE(std)

#endif
