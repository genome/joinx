#include "ConcordanceQuality.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

unsigned ConcordanceQuality::qualityLevel(const Bed& snv) {
    char* end = NULL;
    const string& qualityString = snv.extraFields()[1];
    unsigned qual = strtoul(qualityString.c_str(), &end, 10);
    if (end != &qualityString[qualityString.size()])
        throw runtime_error(str(format("Unable to parse quality value %1%") %qualityString));
    return qual;
}
