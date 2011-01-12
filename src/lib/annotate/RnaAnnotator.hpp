#pragma once

#include "TranscriptAnnotator.hpp"

class RnaAnnotator : public TranscriptAnnotator {
public:
    Region::RelativePos determineCodingPosition(const Variant& v, const TranscriptStructure& structure) const;

};
