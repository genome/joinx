#include "InputStream.hpp"

using namespace std;

InputStream::InputStream(const string& name, istream& s)
    : _name(name)
    , _s(s)
    , _caching(false)
    , _cacheIter(_cache.begin())
{
}

void InputStream::caching(bool value) {
    _caching = value;
}

void InputStream::rewind() {
    _cacheIter = _cache.begin();
}

bool InputStream::getline(string& line) {
    if (_cacheIter != _cache.end()) {
        line = *_cacheIter++;
        return true;
    } 

    std::getline(_s, line);
    if (_caching && _s) {
        _cache.push_back(line);
        _cacheIter = _cache.end();
    }

    return _s;
}

bool InputStream::eof() const {
    return _cacheIter == _cache.end() && _s.eof();
}
