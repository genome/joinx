#pragma once

#include "intconfig.hpp"
#include <string>

struct Bed
{
    enum Type {
        SNV,
        INDEL
    };

    Bed()
        : start(0)
        , end(0)
    {}

    Bed(const std::string& chrom, uint64_t start, uint64_t end, const std::string& refCall, const std::string& qual)
        : chrom(chrom)
        , start(start)
        , end(end)
        , refCall(refCall)
        , qual(qual)
    {}

    std::string chrom;
    uint64_t start;
    uint64_t end;
    std::string refCall;
    std::string qual;

    std::string line;

    int cmp(const Bed& rhs) const;
    bool operator<(const Bed& rhs) const {
        return cmp(rhs) < 0;
    }

    bool type() const {
        return (end == start+1) ? SNV : INDEL;
    }

    static Bed parseLine(const std::string& line);
};
