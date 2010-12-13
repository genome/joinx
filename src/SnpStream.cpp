#include "SnpStream.hpp"

#include "Bed.hpp"
#include "SnpFilterBase.hpp"

using namespace std;

SnpStream::SnpStream(const std::string& name, std::istream& in)
    : _name(name)
    , _in(in)
    , _lineNum(0)
    , _snpCount(0)
{
}

void SnpStream::addFilter(SnpFilterBase* s) {
    _filters.push_back(s);
}

bool SnpStream::eof() const {
    return _in.eof();
}

bool SnpStream::nextSnp(Bed& snp) {
    do {
        string line = nextLine();
        if (line.empty())
            return false;

        snp = Bed::parseLine(line);
    } while (!snp.isSnp() || exclude(snp));
    ++_snpCount;
    return true;
}

string SnpStream::nextLine() {
    string line;
    do {
        getline(_in, line);
        ++_lineNum;
    } while (!eof() && line.empty());
    return line;
}

bool SnpStream::exclude(const Bed& snp) {
    typedef vector<SnpFilterBase*>::iterator IterType;
    for (IterType iter = _filters.begin(); iter != _filters.end(); ++iter) {
        if ((*iter)->exclude(snp))
            return true;
    }
        
    return false;
}
