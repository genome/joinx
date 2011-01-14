#include "FlankAnnotator.hpp"

#include "common/Variant.hpp"
#include "fileformats/TranscriptStructure.hpp"

#include <sstream>

using boost::lexical_cast;
using namespace std;

string FlankAnnotator::codingRegionString(const Variant& v, const TranscriptStructure& structure) const {
    const Region& region = structure.hasCodingRegion() ? structure.codingRegion() : structure.transcriptRegion();

    return region.distance(v.start()).toString();
}



