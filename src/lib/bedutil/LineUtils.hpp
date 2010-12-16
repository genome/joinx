#pragma once

#include "intconfig.hpp"

#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

template<typename IntType>
bool extractInt(const char* start, const char* end, IntType& value) {
    char* realEnd = NULL;
    value = strtoul(start, &realEnd, 10);
    return realEnd == end;
}

inline void extractField(
    const std::string& line,
    uint32_t offset,
    uint32_t skip,
    string::size_type& outBegin,
    string::size_type& outEnd,
    const char delim = '\t'
    )
{
    string::size_type tabPos = offset;
    if (tabPos >= line.size()) {
        stringstream errmsg;
        errmsg << "Attempted to seek past end of line (position " << offset
            << ") '" << line << "'";
        throw runtime_error(errmsg.str());
    }

    for (uint32_t i = 0; i < skip; ++i) {
        tabPos = line.find_first_of(delim, tabPos);
        if (tabPos == string::npos) {
            stringstream errmsg;
            errmsg << "Failed to seek ahead " << skip << " fields in line '"
                << line << "' starting from position " << offset;
            throw runtime_error(errmsg.str());
        }
        ++tabPos;
    }
    outBegin = tabPos;
    outEnd = line.find_first_of(delim, outBegin);
    if (outEnd == string::npos)
        outEnd = line.size(); 
}
