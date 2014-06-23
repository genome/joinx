#pragma once

#include "common/cstdint.hpp"

#include <string>
#include <iostream>

class Sequence {
public:
    static std::string reverseComplement(const std::string& data);

    template<typename IterTypeA, typename IterTypeB>
    static uint64_t commonPrefix(
        IterTypeA aBeg, IterTypeA aEnd,
        IterTypeB bBeg, IterTypeB bEnd)
    {
        uint64_t rv(0);
        while (aBeg != aEnd && bBeg != bEnd && *aBeg++ == *bBeg++)
            ++rv;
        return rv;
    }

    template<typename TA, typename TB>
    static uint64_t commonPrefix(TA const& a, TB const& b) {
        return commonPrefix(a.begin(), a.end(), b.begin(), b.end());
    }

    template<typename Iterator, typename OutputFunc>
    static void findHomopolymers(Iterator begin, Iterator end, OutputFunc& out, ssize_t minLength) {
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
    static void findHomopolymers(StringType const& str, OutputFunc& out, ssize_t minLength) {
        findHomopolymers(str.begin(), str.end(), out, minLength);
    }

    Sequence();
    Sequence(const std::string& data);
    Sequence(char data);
    Sequence(std::istream& s, uint64_t count);

    Sequence& operator=(const std::string& rhs);

    bool null() const;
    bool empty() const;
    const std::string& data() const;
    const std::string& reverseComplementData() const;

    bool operator==(const Sequence& rhs) const;
    bool operator!=(const Sequence& rhs) const;

protected:
    std::string _data;
    mutable std::string _reverseComplement;
};

inline bool Sequence::empty() const {
    return _data.empty();
}

inline const std::string& Sequence::data() const {
    return _data;
}

inline Sequence& Sequence::operator=(const std::string& rhs) {
    _data = rhs;
    return *this;
}

inline bool Sequence::null() const {
    return empty() || _data == "-" || _data == "0" || _data == "*";
}

inline bool Sequence::operator==(const Sequence& rhs) const {
    return _data == rhs._data;
}

inline bool Sequence::operator!=(const Sequence& rhs) const {
    return !(*this == rhs);
}

