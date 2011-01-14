#include "ReferenceSequence.hpp"

#include "common/Sequence.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

ReferenceSequence::ReferenceSequence(const string& dataDir)
    : _dataDir(dataDir)
{}

ReferenceSequence::~ReferenceSequence() {
    for (MapType::iterator iter = _files.begin(); iter != _files.end(); ++iter) {
        iter->second->close();
        delete iter->second;
    }
}

Sequence ReferenceSequence::lookup(const string& chrom, uint64_t start, uint64_t end) {
    ifstream* f = getFileForChrom(chrom);
    f->seekg(start-1);

    Sequence rv(*f, end-start+1); 
    if (!f->good()) {
        throw runtime_error(str(format("Failed to seek to position %1% in chromosome %2%") %(start-1) %chrom));
    }
    return rv;
}

string ReferenceSequence::getPathForChrom(const string& chrom) const {
    return _dataDir + "/" + chrom + ".bases";
}

ifstream* ReferenceSequence::getFileForChrom(const string& chrom) {
    ifstream* p(NULL);
    pair<MapType::iterator, bool> result = _files.insert(make_pair(chrom, p));
    if (result.second) {
        string path = getPathForChrom(chrom);
        result.first->second = new ifstream(path.c_str());
        if (!result.first->second->is_open()) {
            _files.erase(result.first);
            throw runtime_error("Failed to open reference sequence file: " + path);
        }
    }
    return result.first->second;
}
