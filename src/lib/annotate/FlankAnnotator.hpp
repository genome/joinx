#pragma once

#include "TranscriptAnnotator.hpp"

#include <string>

class FlankAnnotator : public TranscriptAnnotator {
public:
    Region::RelativePos determineCodingPosition(const Variant& v, const TranscriptStructure& structure) const;
};
