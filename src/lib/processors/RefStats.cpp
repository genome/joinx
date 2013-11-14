#include "RefStats.hpp"

#include "common/Tokenizer.hpp"
#include "fileformats/Fasta.hpp"
#include "io/StreamJoin.hpp"

#include <boost/format.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <cctype>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using boost::format;

namespace {
    template<typename T>
    bool longer(T const& x, T const& y) {
        return x.size() > y.size();
    }

    bool invalidChar(char c) {
        return !std::isalnum(c);
    }
}

TokenSpec::TokenSpec(std::vector<std::string> const& toks)
    : _origTokens(toks)
{
    for (auto i = toks.begin(); i != toks.end(); ++i) {
        auto const& fullToken = *i;
        std::vector<std::string> components;
        Tokenizer<char>::split(fullToken, '/', std::back_inserter(components));

        for (auto j = components.begin(); j != components.end(); ++j) {
            auto invalid = std::find_if(j->begin(), j->end(), invalidChar);

            if (invalid != j->end()) {
                throw InvalidTokenError(str(format(
                    "Invalid character '%1%' found in token '%2%'"
                    ) % *invalid % fullToken));
            }
            addComponent(*j, fullToken);
        }
    }

    std::stable_sort(_tokens.begin(), _tokens.end(), &longer<std::string>);
}

void TokenSpec::addComponent(std::string const& component, std::string const& fullToken) {
    // transform to uppercase
    std::string ucase(component);
    std::transform(ucase.begin(), ucase.end(), ucase.begin(), ::toupper);

    auto inserted = _tokenMap.insert(std::make_pair(ucase, fullToken));
    if (!inserted.second) {
        throw DuplicateTokenError(str(format(
            "Duplicate element '%1%' in token '%2%' (already specified by '%3%')")
            % component % fullToken % inserted.first->second));
    }

    _tokens.push_back(ucase);
}


std::vector<std::string> const& TokenSpec::tokens() const {
    return _tokens;
}

std::vector<std::string> const& TokenSpec::origTokens() const {
    return _origTokens;
}


std::string const& TokenSpec::tokenFor(std::string const& x) const {
    auto found = _tokenMap.find(x);
    if (found == _tokenMap.end()) {
        throw std::runtime_error(str(format(
            "Could not find matched element %1% in token map!")
            % x
            ));
    }
    return found->second;
}



RefStats::RefStats(std::vector<std::string> const& toks, Fasta& refSeq)
    : _tokenSpec(toks)
    , _refSeq(refSeq)
{
    if (_tokenSpec.tokens().empty()) {
        throw std::runtime_error("RefStats called with empty token list.");
    }

    std::stringstream ssRegex;
    ssRegex << "(" << streamJoin(_tokenSpec.tokens()).delimiter(")|(") << ")";

    _regex = ssRegex.str();
}

auto RefStats::match(std::string const& seq, Region const& region) -> Result {
    Result result;
    auto& counts = result.counts;
    auto& ref = result.referenceString;

    int64_t seqlen = _refSeq.seqlen(seq);

    // tokens[0] is the longest token
    int64_t padding = _tokenSpec.tokens()[0].size() - 1;
    Region paddedRegion(
        std::max(region.begin - padding, 0l),
        std::min(region.end + padding, seqlen)
        );

    ref = _refSeq.sequence(seq, paddedRegion.begin + 1, paddedRegion.size());
    std::string ucref(ref.size(), '\0');
    std::transform(ref.begin(), ref.end(), ucref.begin(), ::toupper);

    boost::match_flag_type flags(boost::match_default);
    boost::sregex_iterator begin(ucref.begin(), ucref.end(), _regex, flags);
    boost::sregex_iterator end;

    Region matchRegion;
    for (; begin != end; ++begin) {
        auto const& result = *begin;
        auto whichToken = result.str();
        int64_t size = whichToken.size();
        matchRegion.begin = paddedRegion.begin + result.position();
        matchRegion.end = matchRegion.begin + size;

        int64_t overlap = region.overlap(matchRegion);
        auto const& actualToken = _tokenSpec.tokenFor(whichToken);
        counts[actualToken] += overlap;
    }

    result.referenceString = result.referenceString.substr(
        region.begin - paddedRegion.begin,
        region.size());

    return result;
}
