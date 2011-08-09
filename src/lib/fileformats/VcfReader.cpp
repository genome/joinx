#include "VcfReader.hpp"

VCF_NAMESPACE_BEGIN

Reader::Reader(const std::string& streamName, std::istream& in)
    : _streamName(streamName)
    , _in(in)
    , _cached(false)
    , _cachedRv(false)
{
    while (getline(_in, _buf) && !_buf.empty() && _buf[0] == '#') {
        if (_buf[1] == '#')
            _header.add(_buf.substr(2));
    }
}

const Header& Reader::header() const {
    return _header;
}

bool Reader::eof() {
    return _buf.empty() && _in.eof();
}

bool Reader::next(Entry& e) {
    if (eof())
        return false;

    if (_buf.empty())
        getline(_in, _buf);

    e = Entry(_buf);
    getline(_in, _buf);

    return true;
}

bool Reader::peek(Entry** e) {
    if (!_cached)
        _cachedRv = next(_cachedEntry);
    if (_cachedRv)
        *e = &_cachedEntry;

    return _cachedRv;
}


VCF_NAMESPACE_END
