#pragma once

#include <boost/functional/hash.hpp>

#include <cassert>
#include <cstring>
#include <ostream>
#include <string>

class StringView {
public:
    StringView();
    StringView(char const* beg);
    StringView(char const* beg, char const* end);

    StringView& operator=(StringView const& rhs);
    void clear();


    void assign(char const* beg, char const* end);
    size_t size() const;
    bool empty() const;

    char operator[](size_t idx) const;

    bool startsWith(char const* s) const;
    bool startsWith(std::string const& s) const;
    bool operator==(StringView const& rhs) const;
    bool operator==(std::string const& rhs) const;
    bool operator==(char const* rhs) const;

    char const* begin() const;
    char const* end() const;

protected:
    char const* _beg;
    char const* _end;
    size_t _size;
};

bool operator==(std::string const& lhs, StringView const& rhs);
bool operator==(char const* lhs, StringView const& rhs);
std::ostream& operator<<(std::ostream& stream, StringView const& str);

inline
StringView::StringView()
    : _beg(0)
    , _end(0)
    , _size(0)
{
}

inline
StringView::StringView(char const* beg)
    : _beg(beg)
    , _size(strlen(beg))
{
    _end = _beg + _size;
    assert(_end >= beg);
}

inline
StringView::StringView(char const* beg, char const* end)
    : _beg(beg)
    , _end(end)
    , _size(end-beg)
{
    assert(_end >= beg);
}

inline
StringView& StringView::operator=(StringView const& rhs) {
    assign(rhs._beg, rhs._end);
    return *this;
}

inline
void StringView::clear() {
    _beg = _end = 0;
    _size = 0;
}

inline
void StringView::assign(char const* beg, char const* end) {
    _beg = beg;
    _end = end;
    _size = end-beg;
}

inline
size_t StringView::size() const {
    return _size;
}

inline
bool StringView::empty() const {
    return _size == 0;
}

inline
char StringView::operator[](size_t idx) const {
    assert(idx < _size);
    return _beg[idx];
}

inline
bool StringView::startsWith(std::string const& s) const {
    return startsWith(s.data());
}

inline
bool StringView::startsWith(char const* s) const {
    char const* p(_beg);
    while (p < _end && *s && *p++ == *s)
        ++s;
    return *s == 0;
}

inline
bool StringView::operator==(StringView const& rhs) const {
    return rhs.size() == _size && strncmp(_beg, rhs._beg, _size) == 0;
}

inline
bool StringView::operator==(std::string const& rhs) const {
    return rhs.size() == _size && rhs.compare(0, _size, _beg) == 0;
}

inline
bool StringView::operator==(char const* rhs) const {
    char const* p(_beg);
    while (p < _end && *rhs && *p++ == *rhs)
        ++rhs;
    return p == _end && *rhs == 0;
}

inline
char const* StringView::begin() const {
    return _beg;
}

inline
char const* StringView::end() const {
    return _end;
}

inline
std::ostream& operator<<(std::ostream& stream, StringView const& str) {
    using namespace std;
    stream.write(str.begin(), str.size());
    return stream;
}

inline
bool operator==(std::string const& lhs, StringView const& rhs) {
    return rhs == lhs;
}

inline
bool operator==(char const* lhs, StringView const& rhs) {
    return rhs == lhs;
}

inline
size_t hash_value(StringView const& sv) {
    return boost::hash_range(sv.begin(), sv.end());
}
