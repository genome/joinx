#include "FlankAnnotator.hpp"

#include "Variant.hpp"
#include "TranscriptStructure.hpp"

using boost::lexical_cast;

using namespace std;

Region::RelativePos FlankAnnotator::determineCodingPosition(const Variant& v, const TranscriptStructure& structure) const {
    const Region& region = structure.hasCodingRegion() ? structure.codingRegion() : structure.transcriptRegion();
    return region.distance(v.start());
}



