#include "CdsExonAnnotator.hpp"

#include "Variant.hpp"
#include "TranscriptStructure.hpp"

using boost::lexical_cast;

using namespace std;

Region::RelativePos CdsExonAnnotator::determineCodingPosition(const Variant& v, const TranscriptStructure& structure) const {
    int64_t start = structure.sequencePosition(v.start());
    int64_t stop = structure.sequencePosition(v.stop());
    int64_t phaseBases = structure.phaseBasesBefore;
    if (structure.hasCodingRegion())
        return structure.codingRegion().distance(v.start());

    // I have found no documentation for this particular case and am coding this to match the old output.
    // If transcript strand is +1, then utr exon is always 3' and prefixed with a '*'. Otherwise, the
    // utr exon is considered 5' and prefixed with a '-'. The position is simply the variant start position
    // TODO ... why is the logic like this? Is this what we want?
    return Region::RelativePos(strand == 1 ? AFTER : BEFORE, v.start());
}
