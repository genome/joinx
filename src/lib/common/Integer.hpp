#pragma once

#include <string>
#include <type_traits>

template<typename T>
std::string integerToBinary(T x, bool leadingZeros = true) {
    bool seenOne = false;
    size_t bits = 8 * sizeof(T);
    std::string rv;
    for (size_t i = 0; i < bits; ++i) {
        size_t bit = bits - i - 1;
        int value = x & (1 << bit);
        if (value) {
            seenOne = true;
            rv.push_back('1');
        }
        else if (leadingZeros || seenOne) {
            rv.push_back('0');
        }
    }
    return rv;
}

// For two variables with unsigned types T and U, T not necessarily equal to
// U, return the distance between them in a variable having the type T or
// U, whichever is the larger type.
//
// Example:
// auto x = unsignedDifference(uint8_t(200), uint16_t(500));
//
// x has type uint16_t and value 300.
template<typename T, typename U>
typename std::enable_if<
        // enable for unsigned types only
          std::is_unsigned<T>::value && std::is_unsigned<U>::value
        , typename std::conditional< // select return type
            sizeof(T) >= sizeof(U)
            , T
            , U
            >::type
        >::type
unsignedDifference(T const& x, U const& y) {
    return x >= y ? x - y : y - x;
}

static_assert(
      std::is_same<std::size_t, decltype(unsignedDifference(size_t(), size_t()))>::value
    , "unsignedDifference(size_t, size_t) -> size_t"
    );

static_assert(
      std::is_same<uint16_t, decltype(unsignedDifference(uint16_t(), uint8_t()))>::value
    , "unsignedDifference(uint16_t, uint8_t) -> uint16_t"
    );

static_assert(
      std::is_same<uint32_t, decltype(unsignedDifference(uint16_t(), uint32_t()))>::value
    , "unsignedDifference(uint16_t, uint32_t) -> uint32_t"
    );

static_assert(
      std::is_same<uint32_t, decltype(unsignedDifference(uint8_t(), uint32_t()))>::value
    , "unsignedDifference(uint8_t, uint32_t) -> uint32_t"
    );
