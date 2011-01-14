#pragma once

#include "common/intconfig.hpp"

#include <algorithm>
#include <cstdlib>
#include <string>

using namespace std;

// Tokenize based on single character delimiter.
// Implemented because C++ iostreams, boost::tokenizer, and boost::split were
// too general purpose (i.e., slow).
class Tokenizer {
public:
    Tokenizer(const std::string& s, char delim = '\t')
        : _s(s)
        , _delim(delim)
        , _pos(0)
        , _end(0)
        , _eofCalls(0) // to support the last field being empty, see eof()
    {
        rewind();
    }

    bool extract(std::string& value);
    bool extract(int8_t&  value) { return extractSigned(value); }
    bool extract(int16_t& value) { return extractSigned(value); }
    bool extract(int32_t& value) { return extractSigned(value); }
    bool extract(int64_t& value) { return extractSigned(value); }
    bool extract(uint8_t&  value) { return extractUnsigned(value); }
    bool extract(uint16_t& value) { return extractUnsigned(value); }
    bool extract(uint32_t& value) { return extractUnsigned(value); }
    bool extract(uint64_t& value) { return extractUnsigned(value); }

    template<typename T>
    bool extractSigned(T& value);
    template<typename T>
    bool extractUnsigned(T& value);

    bool advance();
    // returns # of tokens actually skipped
    uint32_t advance(uint32_t count);
    void rewind();
    bool eof();

protected:
    const std::string& _s;
    char _delim;
    std::string::size_type _pos;
    std::string::size_type _end;
    uint32_t _eofCalls;
};

inline bool Tokenizer::extract(std::string& value) {
    if (eof())
        return false;

    std::string::size_type len = _end-_pos;
    value = _s.substr(_pos, len);
    advance();
    return true;
}

inline uint32_t Tokenizer::advance(uint32_t count) {
    uint32_t i = 0;
    while (i++ < count && advance());
    return i-1;
}

inline bool Tokenizer::advance() {
    if (eof())
        return false;

    if (_pos == _s.size())
        ++_eofCalls;

    _pos = std::min(_s.size(), _end+1);
    _end = std::min(_s.size(), _s.find_first_of(_delim, _pos));
    return true;
}

inline void Tokenizer::rewind() {
    _pos = 0;
    _end = _s.find_first_of(_delim);
    _eofCalls = 0;
}

inline bool Tokenizer::eof() {
    // in order to handle the last field being empty, we don't want to
    // just look at _pos == _s.size(). instead, we need to check if the
    // last char in the string is the delim, and if so, return an empty
    // result, but only once!

    if (_s.empty())
        return true;

    if (_pos == _s.size()) {
        if (_s[_s.size()-1] == _delim && _eofCalls == 0)
            return false;
        else
            return true;
    }

    return false;
}

template<typename T>
bool Tokenizer::extractSigned(T& value) {
    char* realEnd = NULL;
    string::size_type expectedLen =_end-_pos;
    value = strtoll(&_s[_pos], &realEnd, 10);
    ptrdiff_t len = realEnd - &_s[_pos];
    bool rv = len == ptrdiff_t(expectedLen);
    if (rv)
        advance();
    return rv;
}

template<typename T>
bool Tokenizer::extractUnsigned(T& value) {
    char* realEnd = NULL;
    string::size_type expectedLen =_end-_pos;
    value = strtoull(&_s[_pos], &realEnd, 10);
    ptrdiff_t len = realEnd - &_s[_pos];
    bool rv = len == ptrdiff_t(expectedLen);
    if (rv)
        advance();
    return rv;
}
