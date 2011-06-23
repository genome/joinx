#include "FastaReader.hpp"

#include <boost/format.hpp>
#include <cassert>
#include <stdexcept>

using boost::format;
using namespace std;

FastaReader::FastaReader(const std::string& path)
    : _path(path)
    , _len(0)
    , _buf(NULL)
{
    _fai = fai_load(path.c_str());
    if (_fai == NULL)
        throw runtime_error(str(format("Failed to load fasta file: %1%") %path));
}

FastaReader::~FastaReader() {
    if (_buf)
        free(_buf);
    fai_destroy(_fai);
}

char FastaReader::sequence(const std::string& region, int32_t pos) {
    if (region != _region) {
        if (_buf) free(_buf);
        _buf = fai_fetch(_fai, region.c_str(), &_len);
        _region = region;
    }
    if (_len <= pos) {
        throw length_error(str(format("Invalid fasta read request: file %1%, region %2%, position %3%, but region length=%4%") %_path %region %pos %_len));
    }
    return _buf[pos];
}
