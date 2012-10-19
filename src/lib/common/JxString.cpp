#include "JxString.hpp"

#include <algorithm>
#include <iterator>
#include <cstring>
#include <cassert>

JxString::JxString()
    : _beg(0)
    , _end(0)
    , _size(0)
{
}

JxString::JxString(char const* beg)
    : _beg(beg)
    , _size(strlen(beg))
{
    _end = _beg + _size;
    assert(_end >= beg);
}

JxString::JxString(char const* beg, char const* end)
    : _beg(beg)
    , _end(end)
    , _size(end-beg)
{
    assert(_end >= beg);
}

JxString& JxString::operator=(JxString const& rhs) {
    assign(rhs._beg, rhs._end);
    return *this;
}

void JxString::clear() {
    _beg = _end = 0;
    _size = 0;
}

void JxString::assign(char const* beg, char const* end) {
    _beg = beg;
    _end = end;
    _size = end-beg;
}

size_t JxString::size() const {
    return _size;
}

bool JxString::empty() const {
    return _size == 0;
}

char JxString::operator[](size_t idx) const {
    assert(idx < _size);
    return _beg[idx];
}

bool JxString::startsWith(std::string const& s) const {
    return startsWith(s.data());
}

bool JxString::startsWith(char const* s) const {
    char const* p(_beg);
    while (p < _end && *s && *p++ == *s++);
    return *s == 0;
}

bool JxString::operator==(JxString const& rhs) const {
    return rhs.size() == _size && strncmp(_beg, rhs._beg, _size) == 0;
}

bool JxString::operator==(std::string const& rhs) const {
    return rhs.size() == _size && rhs.compare(0, _size, _beg) == 0;
}

bool JxString::operator==(char const* rhs) const {
    char const* p(_beg);
    while (p < _end && *rhs && *p++ == *rhs++);
    return p == _end && *rhs == 0;
}

char const* JxString::begin() const {
    return _beg;
}

char const* JxString::end() const {
    return _end;
}

std::ostream& operator<<(std::ostream& stream, JxString const& str) {
    using namespace std;
    stream.write(str.begin(), str.size());
    return stream;
}

bool operator==(std::string const& lhs, JxString const& rhs) {
    return rhs == lhs;
}

bool operator==(char const* lhs, JxString const& rhs) {
    return rhs == lhs;
}
