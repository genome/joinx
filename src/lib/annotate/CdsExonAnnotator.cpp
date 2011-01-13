#include "CdsExonAnnotator.hpp"

#include "Variant.hpp"
#include "TranscriptStructure.hpp"

#include <sstream>

using boost::lexical_cast;

using namespace std;

string CdsExonAnnotator::codingRegionString(const Variant& v, const TranscriptStructure& structure) const {
    int64_t start = structure.sequencePosition(v.start());
    int64_t stop = structure.sequencePosition(v.end());
    int64_t phaseBases = structure.numPhaseBasesBefore();
    int64_t codingBasesBefore = structure.numCodingBasesBefore();
    stringstream rv;
    if (start != stop)
        rv << (codingBasesBefore + start - phaseBases + 1) <<
            "_" << (codingBasesBefore + stop - phaseBases + 1);
    return rv.str();
}
