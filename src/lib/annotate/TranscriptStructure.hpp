#pragma once

#include "Region.hpp"
#include "bedutil/intconfig.hpp"

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

class TranscriptStructure {
public:
    // TODO: clean up these names, copied from perl
    enum Field {
        transcript_structure_id = 0,
        transcript_id,
        structure_type,
        structure_start,
        structure_stop,
        ordinal,
        phase,
        nucleotide_seq,
        species,
        source,
        version,
        coding_bases_before,
        coding_bases_after,
        cds_exons_before,
        cds_exons_after,
        phase_bases_before,
        phase_bases_after,

        transcript_transcript_id,
        transcript_gene_id,
        transcript_transcript_start,
        transcript_transcript_stop,
        transcript_transcript_name,
        transcript_transcript_status,
        strand,
        transcript_chrom_name,
        transcript_species,
        transcript_source,
        transcript_version,
        transcript_gene_name,
        transcript_transcript_error,
        transcript_coding_region_start,
        transcript_coding_region_stop,
        transcript_amino_acid_length,
        NUM_FIELDS
    };

    static void parseLine(const std::string& line, TranscriptStructure& ts);

    TranscriptStructure();
    const std::string& get(Field f) const;

    template<typename T>
    T getAs(Field f) const {
        return boost::lexical_cast<T>(get(f));
    }

    const std::string& line() const;
    const std::string& chrom() const;

    bool errorContains(const std::string& value) const;

    const Region& region() const;
    const Region& transcriptRegion() const;
    bool hasCodingRegion() const;
    const Region& codingRegion() const;

    int64_t numPhaseBasesBefore() const;
    int64_t sequencePosition(int64_t pos, int64_t& borrowed) const;

protected:
    std::string _line;
    std::string _fields[NUM_FIELDS];

    int _strand;
    Region _region;
    Region _transcriptRegion;
    bool _hasCodingRegion;
    Region _codingRegion;
    int64_t _numPhaseBasesBefore;

private:
    bool (*_strandedLessThan)(int64_t, int64_t);
};

inline const std::string& TranscriptStructure::line() const {
    return _line;
}

inline const std::string& TranscriptStructure::chrom() const {
    return get(transcript_chrom_name);
}

inline const Region& TranscriptStructure::region() const {
    return _region;
}

inline const Region& TranscriptStructure::transcriptRegion() const {
    return _transcriptRegion;
}

inline bool TranscriptStructure::hasCodingRegion() const {
    return _hasCodingRegion;
}

inline const Region& TranscriptStructure::codingRegion() const {
    return _codingRegion;
}

inline int64_t TranscriptStructure::numPhaseBasesBefore() const {
    return _numPhaseBasesBefore;
}
