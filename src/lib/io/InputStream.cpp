#include "InputStream.hpp"

#include "common/compat.hpp"
#include "io/StreamLineSource.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

CompressionType compressionTypeFromString(const string& s) {
    if (s.empty() || s == "n")
        return NONE;
    else if (s == "g")
        return GZIP;
    else
        throw runtime_error(str(format("Invalid compression string '%1%'. Expected one of: n,g,z,b") %s));
}

InputStream::ptr InputStream::create(const string& name, ILineSource::ptr& in) {
    return std::make_unique<InputStream>(name, in);
}

InputStream::ptr InputStream::create(const string& name, istream& in) {
    return std::make_unique<InputStream>(name, in);
}

InputStream::InputStream(const std::string& name, ILineSource::ptr& in)
    : _name(name)
    , _inptr(in.release())
    , _in(*_inptr)
    , _caching(false)
    , _cacheIter(_cache.begin())
    , _lineNum(0)

{
}

InputStream::InputStream(const string& name, istream& in)
    : _name(name)
    , _inptr(std::make_unique<StreamLineSource>(in))
    , _in(*_inptr)
    , _caching(false)
    , _cacheIter(_cache.begin())
    , _lineNum(0)
{
}

void InputStream::caching(bool value) {
    _caching = value;
}

void InputStream::rewind() {
    _lineNum -= distance(_cache.begin(), _cacheIter);
    _cacheIter = _cache.begin();
}

bool InputStream::getline(string& line) {
    if (_cacheIter != _cache.end()) {
        line = *_cacheIter++;
        ++_lineNum;
        return true;
    }

    // read until we get a line that isn't blank.
    while (!_in.eof() && _in.getline(line) && line.empty())
        ++_lineNum;

    ++_lineNum;


    if (_caching && _in) {
        _cache.push_back(line);
        _cacheIter = _cache.end();
    }

    return bool(_in);
}

char InputStream::peek() {
    if (_cacheIter != _cache.end())
        return (*_cacheIter)[0];

    return _in.peek();
}


bool InputStream::eof() const {
    return _cacheIter == _cache.end() && _in.eof();
}

uint64_t InputStream::lineNum() const {
    return _lineNum;
}
