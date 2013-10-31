#pragma once

#include "common/Region.hpp"
#include "common/cstdint.hpp"

#include <boost/regex.hpp>

#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Fasta;

class RefStats {
public:
    struct Result {
        std::string referenceString;
        std::unordered_map<std::string, size_t> counts;
        size_t count(std::string const& x) const {
            auto iter = counts.find(x);
            if (iter != counts.end()) {
                return iter->second;
            }
            return 0;
        }
    };

    RefStats(std::vector<std::string> const& toks, Fasta& refSeq);

    Result match(std::string const& seq, Region const& region);

    template<typename EntryType>
    Result match(EntryType const& entry) {
        return match(entry.chrom(), Region(entry.start(), entry.stop()));
    }

private:
    std::vector<std::string> _tokens;
    Fasta& _refSeq;
    boost::regex _regex;
};
