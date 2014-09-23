#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>
#include <vector>

template<typename IterListType>
struct CommonPrefixMultiImpl {
    enum Status {
          STOP
        , CONTINUE
    };

    CommonPrefixMultiImpl(IterListType begs, IterListType ends)
        : begs(std::move(begs))
        , ends(std::move(ends))
    {
        assert(begs.size() == ends.size());
    }

    Status incrementAll() {
        for (std::size_t i = 0; i < begs.size(); ++i) {
            if (++begs[i] == ends[i]) {
                return STOP;
            }
        }
        return CONTINUE;
    }

    bool anyDone() {
        for (std::size_t i = 0; i < begs.size(); ++i) {
            if (begs[i] == ends[i])
                return true;
        }
        return false;
    }

    uint64_t operator()() {
        uint64_t rv{0};
        if (begs.empty() || anyDone())
            return rv;

        do {
            for (std::size_t i = 1; i < begs.size(); ++i) {
                if (*begs[i] != *begs[0])
                    return rv;
            }
            ++rv;
        } while (incrementAll() == CONTINUE);

        return rv;
    }

    IterListType begs;
    IterListType ends;
};

template<typename StringList>
uint64_t commonPrefixMulti(StringList const& xs) {
    typedef decltype(xs.front().begin()) IterType;
    typedef std::vector<IterType> IterList;
    IterList begs;
    IterList ends;

    for (auto i = xs.begin(); i != xs.end(); ++i) {
        begs.push_back(i->begin());
        ends.push_back(i->end());
    }

    CommonPrefixMultiImpl<IterList> obj(begs, ends);
    return obj();
}

template<typename StringList>
uint64_t commonSuffixMulti(StringList const& xs) {
    typedef decltype(xs.front().rbegin()) IterType;
    typedef std::vector<IterType> IterList;
    IterList begs;
    IterList ends;

    for (auto i = xs.begin(); i != xs.end(); ++i) {
        begs.push_back(i->rbegin());
        ends.push_back(i->rend());
    }

    CommonPrefixMultiImpl<IterList> obj(begs, ends);
    return obj();
}

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
