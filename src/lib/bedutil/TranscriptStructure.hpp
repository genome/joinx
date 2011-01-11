#pragma once

#include "intconfig.hpp"

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>

class TranscriptStructure {
public:
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
        transcript_strand,
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
    int64_t start() const;
    int64_t end() const;

protected:
    std::string _line;
    std::string _fields[NUM_FIELDS];
    int64_t _start;
    int64_t _end;
};

inline const std::string& TranscriptStructure::line() const {
    return _line;
}

inline const std::string& TranscriptStructure::chrom() const {
    return get(transcript_chrom_name);
}

inline int64_t TranscriptStructure::start() const {
    return _start;
}

inline int64_t TranscriptStructure::end() const {
    return _end;
}
