#include "TranscriptAnnotator.hpp"

#include "Variant.hpp"
#include "TranscriptStructure.hpp"

using namespace std;

vector<string> annotate(Variant v, const TranscriptStructure& ts) {
    if (ts.get(TranscriptStructure::strand) == "-1")
        v.reverseComplement();

    vector<string> rv;
    return rv;
}
