#include "BedStream.hpp"

#include "BedFilterBase.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

BedStream::BedStream(const string& name, istream& in, int maxExtraFields /* = -1 */)
    : _name(name)
    , _in(in)
    , _maxExtraFields(maxExtraFields)
    , _lineNum(0)
    , _bedCount(0)
    , _cached(false)
    , _cachedRv(false)
{
    if (!_in.good())
        throw runtime_error(str(format("Input stream '%1%' is invalid") %name));
}

void BedStream::addFilter(BedFilterBase* filter) {
    _filters.push_back(filter);
}

void BedStream::checkEof() const {
    if (!_cached && eof())
        throw runtime_error("Attempted to read past eof of stream " + name());
}

bool BedStream::peek(Bed** bed) {
    // already peeked and have a value to return
    if (_cached) {
        // we peeked but got EOF
        if (_in.eof())
            return false;

        *bed = &_cachedBed;
        return true;
    }

    checkEof();

    // need to peek ahead
    // note: next() may return false (because of EOF). we want to take care
    // when using _cachedBed to make sure that EOF is false.
    _cachedRv = next(_cachedBed);
    *bed = &_cachedBed;
    _cached = true;
    return _cachedRv;
}

bool BedStream::eof() const {
    if (_cached)
        return !_cachedRv;
    else
        return _in.eof();
}

bool BedStream::next(Bed& bed) {
    if (_cached) {
        bed.swap(_cachedBed);
        _cached = false;
        return _cachedRv;
    }

    do {
        string line = nextLine();
        if (line.empty())
            return false;

        Bed::parseLine(line, bed, _maxExtraFields);
    } while (exclude(bed));
    ++_bedCount;
    return true;
}

string BedStream::nextLine() {
    string line;
    do {
        getline(_in, line);
        ++_lineNum;
    } while (!eof() && (line.empty() || line[0] == '#'));
    return line;
}

bool BedStream::exclude(const Bed& bed) {
    typedef vector<BedFilterBase*>::iterator IterType;
    for (IterType iter = _filters.begin(); iter != _filters.end(); ++iter) {
        if ((*iter)->exclude(bed))
            return true;
    }
        
    return false;
}

BedStream& operator>>(BedStream& s, Bed& bed) {
    s.checkEof();
    s.next(bed);
    return s;
}

