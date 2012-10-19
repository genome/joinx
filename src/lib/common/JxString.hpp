#pragma once

#include <cassert>
#include <cstring>
#include <ostream>
#include <string>

class JxString {
public:
    JxString();
    JxString(char const* beg);
    JxString(char const* beg, char const* end);

    JxString& operator=(JxString const& rhs);
    void clear();


    void assign(char const* beg, char const* end);
    size_t size() const;
    bool empty() const;

    char operator[](size_t idx) const;

    bool startsWith(char const* s) const;
    bool startsWith(std::string const& s) const;
    bool operator==(JxString const& rhs) const;
    bool operator==(std::string const& rhs) const;
    bool operator==(char const* rhs) const;

    char const* begin() const;
    char const* end() const;

protected:
    char const* _beg;
    char const* _end;
    size_t _size;
};

bool operator==(std::string const& lhs, JxString const& rhs);
bool operator==(char const* lhs, JxString const& rhs);
std::ostream& operator<<(std::ostream& stream, JxString const& str);

inline
JxString::JxString()
    : _beg(0)
    , _end(0)
    , _size(0)
{
}

inline
JxString::JxString(char const* beg)
    : _beg(beg)
    , _size(strlen(beg))
{
    _end = _beg + _size;
    assert(_end >= beg);
}

inline
JxString::JxString(char const* beg, char const* end)
    : _beg(beg)
    , _end(end)
    , _size(end-beg)
{
    assert(_end >= beg);
}

inline
JxString& JxString::operator=(JxString const& rhs) {
    assign(rhs._beg, rhs._end);
    return *this;
}

inline
void JxString::clear() {
    _beg = _end = 0;
    _size = 0;
}

inline
void JxString::assign(char const* beg, char const* end) {
    _beg = beg;
    _end = end;
    _size = end-beg;
}

inline
size_t JxString::size() const {
    return _size;
}

inline
bool JxString::empty() const {
    return _size == 0;
}

inline
char JxString::operator[](size_t idx) const {
    assert(idx < _size);
    return _beg[idx];
}

inline
bool JxString::startsWith(std::string const& s) const {
    return startsWith(s.data());
}

inline
bool JxString::startsWith(char const* s) const {
    char const* p(_beg);
    while (p < _end && *s && *p++ == *s)
        ++s;
    return *s == 0;
}

inline
bool JxString::operator==(JxString const& rhs) const {
    return rhs.size() == _size && strncmp(_beg, rhs._beg, _size) == 0;
}

inline
bool JxString::operator==(std::string const& rhs) const {
    return rhs.size() == _size && rhs.compare(0, _size, _beg) == 0;
}

inline
bool JxString::operator==(char const* rhs) const {
    char const* p(_beg);
    while (p < _end && *rhs && *p++ == *rhs)
        ++rhs;
    return p == _end && *rhs == 0;
}

inline
char const* JxString::begin() const {
    return _beg;
}

inline
char const* JxString::end() const {
    return _end;
}

inline
std::ostream& operator<<(std::ostream& stream, JxString const& str) {
    using namespace std;
    stream.write(str.begin(), str.size());
    return stream;
}

inline
bool operator==(std::string const& lhs, JxString const& rhs) {
    return rhs == lhs;
}

inline
bool operator==(char const* lhs, JxString const& rhs) {
    return rhs == lhs;
}
