#include "InputStream.hpp"

using namespace std;

InputStream::InputStream(const string& name, istream& s)
    : _name(name)
    , _s(s)
    , _caching(false)
{
}

void InputStream::caching(bool value) {
    _caching = value;
}

bool InputStream::getline(string& line) {
    if (!_caching && !_cache.empty()) {
        line = _cache.front();
        _cache.pop_front();
        return true;
    } 

    std::getline(_s, line);
    if (_caching && _s)
        _cache.push_back(line);
    return _s;
}

bool InputStream::eof() const {
    if (_caching)
        return _s.eof();
    else
        return _cache.empty() && _s.eof();
}
