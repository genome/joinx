#pragma once

#include "common/intconfig.hpp"
#include <algorithm>
#include <string>
#include <sstream>

struct Bed
{
    enum Type {
        SNV,
        INDEL
    };

    Bed()
        : start(0)
        , stop(0)
    {}

    Bed(const std::string& chrom, int64_t start, int64_t stop, const std::string& refCall, const std::string& qual)
        : chrom(chrom)
        , start(start)
        , stop(stop)
        , refCall(refCall)
        , qual(qual)
    {
        std::stringstream ss;
        ss << chrom << "\t" << start << "\t" << stop << "\t" << refCall << "\t"
            << qual;
        line = ss.str();
    }

    std::string chrom;
    int64_t start;
    int64_t stop;
    std::string refCall;
    std::string qual;
    std::string line;

    void swap(Bed& rhs) {
        chrom.swap(rhs.chrom);
        std::swap(start, rhs.start);
        std::swap(stop, rhs.stop);
        refCall.swap(rhs.refCall);
        qual.swap(rhs.qual);
        line.swap(rhs.line);
    }

    int cmp(const Bed& rhs) const;
    bool operator<(const Bed& rhs) const {
        return cmp(rhs) < 0;
    }

    bool operator==(const Bed& rhs) const {
        return cmp(rhs) == 0;
    }

    bool type() const {
        return (stop == start+1) ? SNV : INDEL;
    }

    static void parseLine(std::string& line, Bed& bed);

private:
};

std::ostream& operator<<(std::ostream& s, const Bed& bed);
std::istream& operator>>(std::istream& s, Bed& bed);
