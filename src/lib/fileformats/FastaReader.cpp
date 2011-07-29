#include "FastaReader.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <stdexcept>

using boost::format;
using namespace std;

FastaReader::FastaReader(const string& path)
    : _path(path)
    , _len(0)
    , _buf(NULL)
{
    string failmsg = str(format("Failed to load fasta file: %1%") %path);
    // samtools 0.1.17 segfaults if you fai_load a non-existing file
    ifstream tmp(path.c_str());
    if (!tmp.is_open() || (_fai = fai_load(path.c_str())) == NULL)
        throw runtime_error(failmsg);
}

FastaReader::~FastaReader() {
    if (_buf)
        free(_buf);
    fai_destroy(_fai);
}

void FastaReader::loadRegion(const string& region) {
    if (region != _region) {
        if (_buf) free(_buf);
        _buf = fai_fetch(_fai, region.c_str(), &_len);
        _region = region;
    }
}

char FastaReader::sequence(const string& region, int32_t pos) {
    loadRegion(region);
    if (_len <= pos) {
        throw length_error(str(format("Invalid fasta read request: file %1%, region %2%, position %3%, but region length=%4%") %_path %region %pos %_len));
    }
    return _buf[pos];
}

uint32_t FastaReader::sequence(const string& chrom, int32_t start, int32_t end, string& seq) {
    loadRegion(chrom);
    if (_len <= start) {
        throw length_error(str(format("Invalid fasta read request: file %1%, region %2%, start position %3%, but region length=%4%")
            %_path
            %chrom
            %start
            %_len)
        );
    }

    end = min(end, _len);
    uint32_t len = end-start;
    seq.assign(&_buf[start], len);
    return end;
}
