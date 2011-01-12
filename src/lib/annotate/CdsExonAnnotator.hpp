#pragma once

#include "TranscriptAnnotator.hpp"

#include <string>

class CdsExonAnnotator : public TranscriptAnnotator {
public:
    Region::RelativePos determineCodingPosition(const Variant& v, const TranscriptStructure& structure) const;
};
