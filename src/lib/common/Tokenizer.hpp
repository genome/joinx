#pragma once

#include "JxString.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

using namespace std;

// Implemented because C++ iostreams, boost::tokenizer, and boost::split were
// too general purpose (i.e., slow).
template<typename DelimType>
class Tokenizer {
public:
    Tokenizer(std::string const& s, DelimType const& delim = '\t')
        : _sbeg(s.data())
        , _send(s.data()+s.size())
        , _totalLen(s.size())
        , _delim(delim)
        , _pos(0)
        , _end(0)
        , _eofCalls(0) // to support the last field being empty, see eof()
        , _lastDelim(0)
    {
        rewind();
    }

    Tokenizer(char const* sbeg, char const* send, DelimType const& delim = '\t')
        : _sbeg(sbeg)
        , _send(send)
        , _totalLen(send-sbeg)
        , _delim(delim)
        , _pos(0)
        , _end(0)
        , _eofCalls(0) // to support the last field being empty, see eof()
        , _lastDelim(0)
    {
        rewind();
    }

    Tokenizer(JxString const& s, DelimType const& delim = '\t')
        : _sbeg(s.begin())
        , _send(s.end())
        , _totalLen(s.size())
        , _delim(delim)
        , _pos(0)
        , _end(0)
        , _eofCalls(0) // to support the last field being empty, see eof()
        , _lastDelim(0)
    {
        rewind();
    }

    bool nextTokenMatches(std::string const& value) const {
        return _end-_pos == value.size()
            && strncmp(_sbeg+_pos, value.data(), value.size()) == 0;
    }

    template<typename T>
    bool extract(T& value);

    bool extract(JxString& s) {
        char const* beg;
        char const* end;
        bool rv = _extract(&beg, &end);
        if (rv)
            s.assign(beg, end);
        return rv;
    }

    bool extract(char const** beg, char const** end) {
        return _extract(beg, end);
    }

    template<typename IterType>
    static void split(
        std::string const& s,
        DelimType const& delim,
        IterType v,
        size_t ignoreFirst = 0 // ignore first n tokens
        )
    {
        Tokenizer<DelimType> t(s, delim);
        typename IterType::container_type::value_type tmp;

        while (ignoreFirst-- != 0 && t.extract(tmp));

        while (t.extract(tmp))
            *v++=tmp;
    }

    template<typename IterType>
    static void split(
        char const* beg,
        char const* end,
        DelimType const& delim,
        IterType v,
        size_t ignoreFirst = 0 // ignore first n tokens
        )
    {
        Tokenizer<DelimType> t(beg, end, delim);
        typename IterType::container_type::value_type tmp;

        while (ignoreFirst-- != 0 && t.extract(tmp));

        while (t.extract(tmp))
            *v++=tmp;
    }

    void remaining(std::string& s);

    bool advance();
    // returns # of tokens actually skipped
    uint32_t advance(uint32_t count);
    void rewind();
    bool eof() const;
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
            throw std::runtime_error(str(format("Attempted to cast string '%1%' to char") %s));
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
    bool _extract(long double& value) { return _extractFloat(strtold, value); }
    template<typename T>
    bool _extractSigned(T& value);
    template<typename T>
    bool _extractUnsigned(T& value);
    template<typename T>
    bool _extractFloat(T& value);
    template<typename T>
    bool _extractFloat(T (*func)(const char*, char**), T& value);

    size_t nextDelim();

protected:
    char const* _sbeg;
    char const* _send;
    std::string::size_type _totalLen;
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
    s = _sbeg+_pos;
}

template<typename DelimType>
inline bool Tokenizer<DelimType>::_extract(const char** begin, const char** end) {
    *begin = _sbeg + _pos;
    *end = _sbeg + _end;
    return advance();
}

template<typename DelimType>
inline bool Tokenizer<DelimType>::_extract(std::string& value) {
    std::string::size_type len = _end-_pos;
    value.assign(_sbeg+_pos, len);
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

    _lastDelim = _sbeg[_end];

    if (_pos == _totalLen)
        ++_eofCalls;

    _pos = std::min(_totalLen, _end+1);
    _end = std::min(_totalLen, nextDelim());
    return true;
}

template<typename DelimType>
inline void Tokenizer<DelimType>::rewind() {
    _pos = 0;
    _end = std::min(_totalLen, nextDelim());
    _eofCalls = 0;
}

template<>
inline bool Tokenizer<char>::eof() const {
    // in order to handle the last field being empty, we don't want to
    // just look at _pos == _totalLen. instead, we need to check if the
    // last char in the string is the delim, and if so, return an empty
    // result, but only once!

    if (_sbeg == _send)
        return true;

    if (_pos == _totalLen) {
        if (_sbeg[_totalLen-1] == _delim && _eofCalls == 0)
            return false;
        else
            return true;
    }

    return false;
}

template<>
inline bool Tokenizer<std::string>::eof() const {
    // in order to handle the last field being empty, we don't want to
    // just look at _pos == _totalLen. instead, we need to check if the
    // last char in the string is the delim, and if so, return an empty
    // result, but only once!

    if (_totalLen == 0)
        return true;

    if (_pos == _totalLen) {
        if (_delim.find_first_of(_sbeg[_totalLen-1]) != std::string::npos &&
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
    value = strtoll(&_sbeg[_pos], &realEnd, 10);
    ptrdiff_t len = realEnd - &_sbeg[_pos];
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
    value = strtoull(&_sbeg[_pos], &realEnd, 10);
    ptrdiff_t len = realEnd - &_sbeg[_pos];
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
    value = func(&_sbeg[_pos], &realEnd);
    ptrdiff_t len = realEnd - &_sbeg[_pos];
    bool rv = len == ptrdiff_t(expectedLen);
    if (rv)
        advance();
    return rv;
}

template<>
inline size_t Tokenizer<char>::nextDelim() {
    if (_totalLen == 0) return 0;
    char const* rv(0);
    rv = strchr(_sbeg+_pos, _delim);
    return rv == 0 ? std::string::npos : rv-_sbeg;
}

template<>
inline size_t Tokenizer<std::string>::nextDelim() {
    if (_totalLen == 0) return 0;
    char const* rv(0);
    rv = strpbrk(_sbeg+_pos, _delim.data());
    return rv == 0 ? std::string::npos : rv-_sbeg;
}
