#pragma once

#include <cstdint>

template<typename IterTypeA, typename IterTypeB>
uint64_t commonPrefix(
    IterTypeA aBeg, IterTypeA aEnd,
    IterTypeB bBeg, IterTypeB bEnd)
{
    uint64_t rv(0);
    while (aBeg != aEnd && bBeg != bEnd && *aBeg++ == *bBeg++)
        ++rv;
    return rv;
}

template<typename TA, typename TB>
uint64_t commonPrefix(TA const& a, TB const& b) {
    return commonPrefix(a.begin(), a.end(), b.begin(), b.end());
}

template<typename Iterator, typename OutputFunc>
void findHomopolymers(Iterator begin, Iterator end, OutputFunc& out, int64_t minLength) {
    auto orig = begin;
    auto last = begin;
    while (begin != end) {
        while (*++last == *begin && begin != end)
            ;

        if (last - begin >= minLength) {
            out(begin - orig, last - orig, *begin);
        }

        begin = last;
    }
}

template<typename StringType, typename OutputFunc>
static void findHomopolymers(StringType const& str, OutputFunc& out, int64_t minLength) {
    findHomopolymers(str.begin(), str.end(), out, minLength);
}
