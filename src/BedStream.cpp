#include "BedStream.hpp"

#include "Bed.hpp"
#include "BedFilterBase.hpp"

using namespace std;

BedStream::BedStream(const std::string& name, std::istream& in)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _bedCount(0)
{
}

void BedStream::addFilter(BedFilterBase* s) {
    _filters.push_back(s);
}

bool BedStream::eof() const {
    return _in.eof();
}

bool BedStream::next(Bed& bed) {
    do {
        string line = nextLine();
        if (line.empty())
            return false;

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
