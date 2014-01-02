#pragma once

#include "StringView.hpp"
#include "common/cstdint.hpp"

#include <boost/format.hpp>
#include <boost/spirit/include/qi.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std;

// Implemented because C++ iostreams, boost::tokenizer, and boost::split were
// too general purpose (i.e., slow).
template<typename DelimType>
class Tokenizer {
private: // details...
    // Get value type from normal iterators
    template<typename IterType, typename ValueType>
    struct IteratorValue_impl {
        typedef ValueType value_type;
    };

    // Get value type from output iterators
    template<typename IterType>
    struct IteratorValue_impl<IterType, void> {
        typedef typename IterType::container_type::value_type value_type;
    };

    template<typename T>
    struct IteratorValue {
        typedef typename IteratorValue_impl<
                T,
                typename std::iterator_traits<T>::value_type
                >::value_type
                value_type;
    };

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

    Tokenizer(StringView const& s, DelimType const& delim = '\t')
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

    bool extract(StringView& s) {
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
        char const* beg,
        char const* end,
        DelimType const& delim,
        IterType v
        )
    {
        Tokenizer<DelimType> t(beg, end, delim);
        typedef typename IteratorValue<IterType>::value_type ValueType;
        ValueType tmp;

        while (t.extract(tmp))
            *v++ = std::move(tmp);
    }

    template<typename IterType>
    static void split(
        StringView const& s,
        DelimType const& delim,
        IterType v
        )
    {
        return split<IterType>(s.begin(), s.end(), delim, v);
    }

    template<typename IterType>
    static void split(
        std::string const& s,
        DelimType const& delim,
        IterType v
        )
    {
        return split<IterType>(s.data(), s.data() + s.size(), delim, v);
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
        value = std::move(s);
        return true;
    }

    // special case for casting string to char
    bool _extract(char& value) {
        bool rv = _end - _pos == 1;
        if (rv) {
            value = _sbeg[_pos];
            advance();
        }
        return rv;
    }

    bool _extract(std::string& value);

    template<typename T, typename V>
    bool _extractNumeric(T& parser, V& value);


    bool _extract(const char** begin, const char** end);


    bool _extract(int16_t& value) {
        return _extractNumeric(boost::spirit::qi::short_, value);
    }

    bool _extract(int32_t& value) {
        return _extractNumeric(boost::spirit::qi::int_, value);
    }

    bool _extract(int64_t& value) {
        return _extractNumeric(boost::spirit::qi::long_, value);
    }

    bool _extract(uint16_t& value) {
        return _extractNumeric(boost::spirit::qi::ushort_, value);
    }

    bool _extract(uint32_t& value) {
        return _extractNumeric(boost::spirit::qi::uint_, value);
    }

    bool _extract(uint64_t& value) {
        return _extractNumeric(boost::spirit::qi::ulong_, value);
    }


    bool _extract(float& value) {
        return _extractNumeric(boost::spirit::qi::float_, value);
    }

    bool _extract(double& value) {
        return _extractNumeric(boost::spirit::qi::double_, value);
    }

    bool _extract(long double& value) {
        return _extractNumeric(boost::spirit::qi::long_double, value);
    }

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
template<typename T, typename V>
inline bool Tokenizer<DelimType>::_extractNumeric(T& parser, V& value) {
    namespace qi = boost::spirit::qi;
    auto beg = &_sbeg[_pos];
    auto end = &_sbeg[_end];

    qi::parse(beg, end, parser, value);

    //value = func(&_sbeg[_pos], &realEnd);
    bool rv = beg == end;
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
