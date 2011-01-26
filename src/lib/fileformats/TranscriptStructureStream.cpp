#include "TranscriptStructureStream.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std;

TranscriptStructureStream::TranscriptStructureStream(const string& name, istream& in)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _cached(false)
{
}

void TranscriptStructureStream::checkEof() const {
    if (eof())
        throw runtime_error("Attempted to read past eof of stream " + name());
}

bool TranscriptStructureStream::peek(TranscriptStructure** structure) {
    // already peeked and have a value to return
    if (_cached) {
        // we peeked but got EOF
        if (_in.eof())
            return false;

        *structure = &_cachedTranscriptStructure;
        return true;
    }

    checkEof();

    // need to peek ahead
    // note: next() may return false (because of EOF). we want to take care
    // when using _cachedTranscriptStructure to make sure that EOF is false.
    bool rv = next(_cachedTranscriptStructure);
    *structure = &_cachedTranscriptStructure;
    _cached = true;
    return rv;
}

bool TranscriptStructureStream::eof() const {
    return !_cached && _in.eof();
}

bool TranscriptStructureStream::next(TranscriptStructure& structure) {
    if (_cached) {
        swap(structure, _cachedTranscriptStructure);
        _cached = false;
        return !eof(); // to handle the case where we peeked at EOF
    }

    string line = nextLine();
    if (line.empty())
        return false;

    cout << "LINE: " << line << "\n";
    TranscriptStructure::parseLine(line, structure);
    return true;
}

string TranscriptStructureStream::nextLine() {
    string line;
    do {
        getline(_in, line);
        ++_lineNum;
    } while (!eof() && line.empty());
    return line;
}
