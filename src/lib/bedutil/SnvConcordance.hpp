#pragma once

#include "common/intconfig.hpp"

#include <iostream>
#include <map>
#include <string>

class Bed;
class Variant;

enum ZygosityType {
    HOMOZYGOUS,
    HETEROZYGOUS
};

enum SnvType {
    AMBIGUOUS,
    REFERENCE,
    SNV
};

enum MatchType {
    MATCH,
    PARTIAL_MATCH,
    MISMATCH,
    REFERENCE_MISMATCH
};

struct Zygosity {
    ZygosityType type;
    unsigned alleleCount;
};

struct SnvDescription {
    SnvDescription() : overlap(false) {}

    Zygosity zygosity;
    SnvType type;
    unsigned overlap;

    std::string category() const;
    std::string toString(bool detail = true) const;
};

struct MatchDescription {
    MatchType matchType;
    SnvDescription descA;
    SnvDescription descB;
};

class SnvConcordance {
public:
    SnvConcordance();

    void missA(const Bed& a);
    void missB(const Bed& b);
    bool hit(const Bed& a, const Bed& b);

    void report(std::ostream& s);

    // NOTE: input strings must be sorted lexically (i.e., ACGT)
    static unsigned overlap(const std::string& sa, const std::string& sb);
    static Zygosity zygosity(const std::string& callIub);
    static SnvDescription describeSnv(const std::string& refIub, const std::string& callIub);
    static MatchDescription matchDescription(const Variant& a, const Variant& b);
    static std::string matchTypeString(MatchType type);

protected:
    struct ResultCounter {
        uint64_t total;
        uint64_t hits;
        uint64_t depth;

        ResultCounter& operator+=(const ResultCounter& rhs) {
            total += rhs.total;
            hits += rhs.hits;
            depth += rhs.depth;
            return *this;
        }
    };

    void updateResult(const MatchDescription& md, const ResultCounter& rc);

protected:
    typedef std::map<std::string, ResultCounter> MapDescToCount;
    typedef std::map<MatchType, MapDescToCount> MapMatchTypeToDescCount;
    typedef std::map<std::string, MapMatchTypeToDescCount> MapType;
    MapType _results;
};

inline Zygosity SnvConcordance::zygosity(const std::string& callIub) {
    Zygosity rv = {
        callIub.size() > 1 ? HETEROZYGOUS : HOMOZYGOUS,
        callIub.size()
    };
    return rv;
}

