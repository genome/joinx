#include "ReferenceSequence.hpp"

#include "common/Sequence.hpp"
#include "fileformats/BasesFile.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

ReferenceSequence::ReferenceSequence(const string& dataDir)
    : _dataDir(dataDir)
{}

ReferenceSequence::~ReferenceSequence() {
    for (MapType::iterator iter = _files.begin(); iter != _files.end(); ++iter)
        delete iter->second;
}

Sequence ReferenceSequence::lookup(const string& chrom, uint64_t start, uint64_t end) {
    return getFileForChrom(chrom)->lookup(start, end);;
}

string ReferenceSequence::getPathForChrom(const string& chrom) const {
    return _dataDir + "/" + chrom + ".bases";
}

BasesFile* ReferenceSequence::getFileForChrom(const string& chrom) {
    BasesFile* p(NULL);
    pair<MapType::iterator, bool> result = _files.insert(make_pair(chrom, p));
    if (result.second) {
        string path = getPathForChrom(chrom);

        try {
            result.first->second = new BasesFile(path);
        } catch (...) {
            // make sure to clean up the map if we didn't get the bases file
            _files.erase(result.first);
            throw; // just rethrow whatever we caught
        }
    }
    return result.first->second;
}
