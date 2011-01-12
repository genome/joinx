#pragma once

#include "Region.hpp"
#include <vector>
#include <string>

class Variant;
class TranscriptStructure;

struct Annotation {

    Annotation()
        : trv_type("NULL")
        , c_position("NULL")
        , amino_acid_length("NULL")
        , amino_acid_change("NULL")
    {}

    std::string trv_type;
    std::string c_position;
    std::string amino_acid_length;
    std::string amino_acid_change;
};


class TranscriptAnnotator {
public:
    virtual ~TranscriptAnnotator() {}

    virtual Region::RelativePos determineCodingRegion(const Variant& v, const TranscriptStructure& structure) const = 0;
    virtual void annotate(const Variant& v, const TranscriptStructure& structure) const = 0;
};
