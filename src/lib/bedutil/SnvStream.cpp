#include "SnvStream.hpp"

#include "Bed.hpp"
#include "BedFilterBase.hpp"

using namespace std;

SnvStream::SnvStream(const std::string& name, std::istream& in)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _snvCount(0)
{
}

void SnvStream::addFilter(BedFilterBase* s) {
    _filters.push_back(s);
}

bool SnvStream::eof() const {
    return _in.eof();
}

bool SnvStream::nextSnv(Bed& snv) {
    do {
        string line = nextLine();
        if (line.empty())
            return false;

        snv = Bed::parseLine(line);
    } while (!snv.isSnv() || exclude(snv));
    ++_snvCount;
    return true;
}

string SnvStream::nextLine() {
    string line;
    do {
        getline(_in, line);
        ++_lineNum;
    } while (!eof() && line.empty());
    return line;
}

bool SnvStream::exclude(const Bed& snv) {
    typedef vector<BedFilterBase*>::iterator IterType;
    for (IterType iter = _filters.begin(); iter != _filters.end(); ++iter) {
        if ((*iter)->exclude(snv))
            return true;
    }
        
    return false;
}
