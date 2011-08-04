#pragma once

#include <cstdint>
#include <string>

class Sequence;

class ISequenceReader {
public:
    virtual ~ISequenceReader() {}
    virtual Sequence lookup(const std::string& chrom, int64_t start, int64_t end) = 0;
};
