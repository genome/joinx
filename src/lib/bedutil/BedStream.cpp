#include "BedStream.hpp"

#include "Bed.hpp"
#include "BedFilterBase.hpp"

#include <stdexcept>

using namespace std;

BedStream::BedStream(const string& name, istream& in)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _bedCount(0)
    , _cached(false)
{
}

BedStream::BedStream(const string& name, istream& in, BedFilterBase* filter)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _bedCount(0)
    , _cached(false)
{
    _filters.push_back(filter);
}

BedStream::BedStream(const string& name, istream& in, const vector<BedFilterBase*>& filters)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _bedCount(0)
    , _filters(filters)
    , _cached(false)
{
}

void BedStream::checkEof() const {
    if (eof())
        throw runtime_error("Attempted to read past eof of stream " + name());
}

bool BedStream::peek(Bed& bed) {
    // already peeked and have a value to return
    if (_cached) {
        // we peeked but got EOF
        if (_in.eof())
            return false;

        bed = _cachedBed;
        return true;
    }

    checkEof();

    // need to peek ahead
    // note: next() may return false (because of EOF). we want to take care
    // when using _cachedBed to make sure that EOF is false.
    bool rv = next(_cachedBed);
    bed = _cachedBed;
    _cached = true;
    return rv;
}

bool BedStream::eof() const {
    return !_cached && _in.eof();
}

bool BedStream::next(Bed& bed) {
    if (_cached) {
        bed = _cachedBed;
        _cached = false;
        return !eof(); // to handle the case where we peeked at EOF
    }

    do {
        string line = nextLine();
        if (line.empty()) {
            return false;
        }

        bed = Bed::parseLine(line);
    } while (exclude(bed));
    ++_bedCount;
    return true;
}

string BedStream::nextLine() {
    string line;
    do {
        getline(_in, line);
        ++_lineNum;
    } while (!eof() && line.empty());
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

