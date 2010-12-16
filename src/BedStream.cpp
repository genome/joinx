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
    , _good(true)
    , _lastGood(true)
{
    advance();
}

BedStream::BedStream(const string& name, istream& in, BedFilterBase* filter)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _bedCount(0)
    , _good(true)
    , _lastGood(true)
{
    _filters.push_back(filter);
    advance();
}

BedStream::BedStream(const string& name, istream& in, const vector<BedFilterBase*>& filters)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _bedCount(0)
    , _filters(filters)
    , _good(true)
    , _lastGood(true)
{
    advance();
}

const Bed& BedStream::peek() const {
    if (!_good)
        throw runtime_error("Failed while reading from stream " + name());
    return _bed;
}


bool BedStream::eof() const {
    return !_good || _in.eof();
}

void BedStream::advance() {
    _lastGood = _good;
    do {
        string line = nextLine();
        if (line.empty()) {
            _good = false;
            return;
        }

        _bed = Bed::parseLine(line);
    } while (exclude(_bed));
    ++_bedCount;
    _good = true;
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
    if (s.eof())
        throw runtime_error("Attempted to read past eof of stream " + s.name());
    bed = s.peek();
    s.advance();
    return s;
}

