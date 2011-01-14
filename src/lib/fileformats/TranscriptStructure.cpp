// TODO: get rid of getAs<>/lexical_cast calls as possible

#include "TranscriptStructure.hpp"
#include "common/Tokenizer.hpp"

#include <algorithm>
#include <cassert>
#include <functional>

using boost::lexical_cast;
using namespace std;

namespace {
    bool lessThan(int64_t pos1, int64_t pos2) { return pos1 < pos2; }
    bool greaterThan(int64_t pos1, int64_t pos2) { return pos1 > pos2; }
}

void TranscriptStructure::parseLine(const string& line, TranscriptStructure& ts) {
    
    Tokenizer tokenizer(line, ',');
    unsigned idx = 0;
    while (idx < NUM_FIELDS && tokenizer.extractString(ts._fields[idx++]));
    assert(idx <= NUM_FIELDS);
    ts._line = line;

    if (ts.get(strand) == "+1") {
        ts._strandedLessThan = &lessThan;
        ts._strand = 1;
    } else if (ts.get(strand) == "-1") {
        ts._strandedLessThan = &greaterThan;
        ts._strand = -1;
    } else
        throw runtime_error("invalid strand! " + ts.get(strand));

    ts._region = Region(
        ts._strand,
        ts.getAs<int64_t>(structure_start),
        ts.getAs<int64_t>(structure_stop)
    );

    ts._transcriptRegion = Region(
        ts._strand,
        ts.getAs<int64_t>(transcript_transcript_start),
        ts.getAs<int64_t>(transcript_transcript_stop)
    );

    ts._hasCodingRegion = ts.get(transcript_coding_region_start) != "NULL" &&
        ts.get(transcript_coding_region_stop) != "NULL";

    if (ts._hasCodingRegion) {
        ts._codingRegion = Region(
            ts._strand,
            ts.getAs<int64_t>(transcript_coding_region_start),
            ts.getAs<int64_t>(transcript_coding_region_stop)
        );
    }

    const string& phaseBases = ts.get(phase_bases_before);
    if (phaseBases == "NULL")
        ts._numPhaseBasesBefore = 0;
    else
        ts._numPhaseBasesBefore = phaseBases.size();

    const string& codingBases = ts.get(coding_bases_before);
    if (codingBases == "NULL")
        ts._numCodingBasesBefore = 0;
    else
        ts._numCodingBasesBefore = ts.getAs<int64_t>(coding_bases_before);

    if (ts.get(cds_exons_before) != "NULL")
        ts._cdsExonsBefore = ts.getAs<int64_t>(cds_exons_before);

    if (ts.get(cds_exons_after) != "NULL")
        ts._cdsExonsAfter = ts.getAs<int64_t>(cds_exons_after);
}

TranscriptStructure::TranscriptStructure()
    : _hasCodingRegion(false)
{}

const string& TranscriptStructure::get(Field f) const {
    assert(f < NUM_FIELDS);
    return _fields[f];
}

bool TranscriptStructure::errorContains(const std::string& value) const {
    return get(transcript_transcript_error).find(value) != string::npos;
}

int64_t TranscriptStructure::sequencePosition(int64_t pos, int64_t& borrowed) const {
    return numPhaseBasesBefore() + abs(region().strandedStart() - pos); 
}
