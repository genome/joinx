#pragma once

#include <boost/format.hpp>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>

using namespace std;

// Implemented because C++ iostreams, boost::tokenizer, and boost::split were
// too general purpose (i.e., slow).
template<typename DelimType>
class Tokenizer {
public:
    Tokenizer(const std::string& s, DelimType delim = '\t')
        : _s(s)
        , _delim(delim)
        , _pos(0)
        , _end(0)
        , _eofCalls(0) // to support the last field being empty, see eof()
        , _lastDelim(0)
    {
        rewind();
    }

    void reset(const std::string& s) {
        _s = s;
        _pos = 0;
        _end = 0;
        _eofCalls = 0;
        _lastDelim = 0;
    }

    template<typename T>
    bool extract(T& value);
    void remaining(std::string& s);
    
    bool advance();
    // returns # of tokens actually skipped
    uint32_t advance(uint32_t count);
    void rewind();
    bool eof();
    const char& lastDelim() { return _lastDelim; }

protected:
    template<typename T>
    bool _extract(T& value) {
        std::string s;
        if (!extract(s))
            return false;
        value = T(s);
        return true;
    }

    // special case for casting string to char
    bool _extract(char& value) {
        string s;
        if (! extract(s))
            return false;
        if (s.size() == 1) {
            value = s[0];
            return true;
        } else {
            using boost::format;
            throw std::runtime_error(str(format("Attempted to cast string '%1%' to char") %value));
        }
        return false;
    }

    bool _extract(std::string& value);
    bool _extract(const char** begin, const char** end);
    bool _extract(int8_t&  value) { return _extractSigned(value); }
    bool _extract(int16_t& value) { return _extractSigned(value); }
    bool _extract(int32_t& value) { return _extractSigned(value); }
    bool _extract(int64_t& value) { return _extractSigned(value); }
    bool _extract(uint8_t&  value) { return _extractUnsigned(value); }
    bool _extract(uint16_t& value) { return _extractUnsigned(value); }
    bool _extract(uint32_t& value) { return _extractUnsigned(value); }
    bool _extract(uint64_t& value) { return _extractUnsigned(value); }
    bool _extract(float& value) { return _extractFloat(strtof, value); }
    bool _extract(double& value) { return _extractFloat(strtod, value); }
    template<typename T>
    bool _extractSigned(T& value);
    template<typename T>
    bool _extractUnsigned(T& value);
    template<typename T>
    bool _extractFloat(T& value);
    template<typename T>
    bool _extractFloat(T (*func)(const char*, char**), T& value);


protected:
    const std::string& _s;
    DelimType _delim;
    std::string::size_type _pos;
    std::string::size_type _end;
    uint32_t _eofCalls;
    char _lastDelim;
};

template<typename DelimType>
template<typename T>
inline bool Tokenizer<DelimType>::extract(T& value) {
    if (eof())
        return false;
    return _extract(value);
}

template<typename DelimType>
inline void Tokenizer<DelimType>::remaining(std::string& s) {
    s = _s.substr(_pos);
}
 
template<typename DelimType>
inline bool Tokenizer<DelimType>::_extract(const char** begin, const char** end) {
    *begin = _s.data() + _pos;
    *end = _s.data()+_end;
    advance();
    return true;
}

template<typename DelimType>
inline bool Tokenizer<DelimType>::_extract(std::string& value) {

    std::string::size_type len = _end-_pos;
    value = _s.substr(_pos, len);
    advance();
    return true;
}

template<typename DelimType>
inline uint32_t Tokenizer<DelimType>::advance(uint32_t count) {
    uint32_t i = 0;
    while (i++ < count && advance());
    return i-1;
}

template<typename DelimType>
inline bool Tokenizer<DelimType>::advance() {
    if (eof())
        return false;

    _lastDelim = _s[_end];

    if (_pos == _s.size())
        ++_eofCalls;

    _pos = std::min(_s.size(), _end+1);
    _end = std::min(_s.size(), _s.find_first_of(_delim, _pos));
    return true;
}

template<typename DelimType>
inline void Tokenizer<DelimType>::rewind() {
    _pos = 0;
    _end = std::min(_s.size(), _s.find_first_of(_delim, _pos));
    _eofCalls = 0;
}

template<>
inline bool Tokenizer<char>::eof() {
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

template<>
inline bool Tokenizer<std::string>::eof() {
    // in order to handle the last field being empty, we don't want to
    // just look at _pos == _s.size(). instead, we need to check if the
    // last char in the string is the delim, and if so, return an empty
    // result, but only once!

    if (_s.empty())
        return true;

    if (_pos == _s.size()) {
        if (_delim.find_first_of(_s[_s.size()-1]) != std::string::npos && 
            _eofCalls == 0)
            return false;
        else
            return true;
    }

    return false;
}

template<typename DelimType>
template<typename T>
inline bool Tokenizer<DelimType>::_extractSigned(T& value) {
    char* realEnd = NULL;
    string::size_type expectedLen =_end-_pos;
    value = strtoll(&_s[_pos], &realEnd, 10);
    ptrdiff_t len = realEnd - &_s[_pos];
    bool rv = len == ptrdiff_t(expectedLen);
    if (rv)
        advance();
    return rv;
}

template<typename DelimType>
template<typename T>
inline bool Tokenizer<DelimType>::_extractUnsigned(T& value) {
    char* realEnd = NULL;
    string::size_type expectedLen =_end-_pos;
    value = strtoull(&_s[_pos], &realEnd, 10);
    ptrdiff_t len = realEnd - &_s[_pos];
    bool rv = len == ptrdiff_t(expectedLen);
    if (rv)
        advance();
    return rv;
}

template<typename DelimType>
template<typename T>
inline bool Tokenizer<DelimType>::_extractFloat(T (*func)(const char*, char**), T& value) {
    char* realEnd = NULL;
    string::size_type expectedLen =_end-_pos;
    value = func(&_s[_pos], &realEnd);
    ptrdiff_t len = realEnd - &_s[_pos];
    bool rv = len == ptrdiff_t(expectedLen);
    if (rv)
        advance();
    return rv;
}
