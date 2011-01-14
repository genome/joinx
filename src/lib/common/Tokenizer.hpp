#pragma once

#include "intconfig.hpp"

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

// Tokenize based on single character delimiter.
class Tokenizer {
public:
    explicit Tokenizer(const std::string& s, char delim = '\t')
        : _s(s)
        , _delim(delim)
        , _pos(0)
        , _end(0)
    {
        rewind();
    }

    bool extractString(std::string& value) {
        using std::string;

        if (eof())
            return false;

        string::size_type len = _end-_pos;
        value = _s.substr(_pos, len);
        advance();
        return true;
    }

    template<typename T>
    bool extractSigned(T& value) {
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
    bool extractUnsigned(T& value) {
        char* realEnd = NULL;
        string::size_type expectedLen =_end-_pos;
        value = strtoll(&_s[_pos], &realEnd, 10);
        ptrdiff_t len = realEnd - &_s[_pos];
        bool rv = len == ptrdiff_t(expectedLen);
        if (rv)
            advance();
        return rv;
    }

    // returns # of tokens actually skipped
    uint32_t advance(uint32_t count) {
        uint32_t i = 0;
        while (i++ < count && advance());
        return i-1;
    }

    bool advance() {
        _pos = std::min(_s.size(), _end+1);
        _end = std::min(_s.size(), _s.find_first_of(_delim, _pos));
        return !eof();
    }

    void rewind() {
        _pos = 0;        
        _end = _s.find_first_of(_delim);
    }

    bool eof() {
        return _pos == _s.size();
    }

protected:
    const std::string& _s;
    char _delim;
    std::string::size_type _pos;
    std::string::size_type _end;
};
