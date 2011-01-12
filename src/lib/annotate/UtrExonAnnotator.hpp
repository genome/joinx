#pragma once

#include "TranscriptAnnotator.hpp"

#include <string>

class UtrExonAnnotator : public TranscriptAnnotator {
public:
    Region::RelativePos determineCodingPosition(const Variant& v, const TranscriptStructure& structure) const;
};
