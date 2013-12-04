#pragma once

#include <string>

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
