#pragma once

#include "common/Region.hpp"
#include "common/cstdint.hpp"

#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>

#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class Fasta;

class InvalidTokenError : public std::runtime_error {
public:
    explicit InvalidTokenError(std::string const& message)
        : std::runtime_error(message)
    {
    }
};

class DuplicateTokenError : public std::runtime_error {
public:
    explicit DuplicateTokenError(std::string const& message)
        : std::runtime_error(message)
    {
    }
};



class TokenSpec {
public:
    explicit TokenSpec(std::vector<std::string> const& toks);

    std::string const& tokenFor(std::string const& x) const;

    std::vector<std::string> const& tokens() const;
    std::vector<std::string> const& origTokens() const;

private:
    void addComponent(std::string const& component, std::string const& fullToken);

private:
    std::vector<std::string> const& _origTokens;
    std::vector<std::string> _tokens;
    boost::unordered_map<std::string, std::string> _tokenMap;
};

class RefStats {
public:

    struct Result {
        std::string referenceString;
        boost::unordered_map<std::string, size_t> counts;
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
    TokenSpec _tokenSpec;
    Fasta& _refSeq;
    boost::regex _regex;
};
