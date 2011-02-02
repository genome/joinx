#include "BedStream.hpp"

#include "BedFilterBase.hpp"

#include <stdexcept>

using namespace std;

BedStream::BedStream(const string& name, istream& in, int maxExtraFields /* = -1 */)
    : _name(name)
    , _in(in)
    , _maxExtraFields(maxExtraFields)
    , _lineNum(0)
    , _bedCount(0)
    , _cached(false)
{
}

void BedStream::addFilter(BedFilterBase* filter) {
    _filters.push_back(filter);
}

void BedStream::checkEof() const {
    if (eof())
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
    bool rv = next(_cachedBed);
    *bed = &_cachedBed;
    _cached = true;
    return rv;
}

bool BedStream::eof() const {
    return !_cached && _in.eof();
}

bool BedStream::next(Bed& bed) {
    if (_cached) {
        bed.swap(_cachedBed);
        _cached = false;
        return !eof(); // to handle the case where we peeked at EOF
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

