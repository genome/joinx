#include "BasesFile.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

BasesFile::BasesFile(const string& path)
    : _file(path.c_str())
{
    if (!_file)
        throw runtime_error(str(format("Unable to open bases file '%1%'") %path));
}

Sequence BasesFile::lookup(uint64_t start, uint64_t end) {
    _file.seekg(start-1);

    Sequence rv(_file, end-start+1); 
    if (!_file.good())
        throw runtime_error(str(format("Failed to seek to position %1% in bases file %2%") %(start-1) %path()));
    return rv;
}
