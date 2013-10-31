#include "RefStats.hpp"

#include "fileformats/Fasta.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>

namespace {
    template<typename T>
    bool longer(T const& x, T const& y) {
        return x.size() > y.size();
    }
}

RefStats::RefStats(std::vector<std::string> const& toks, Fasta& refSeq)
    : _tokens(toks)
    , _refSeq(refSeq)
{
    if (_tokens.empty()) {
        throw std::runtime_error("RefStats called with empty token list.");
    }

    std::sort(_tokens.begin(), _tokens.end(), &longer<std::string>);

    std::stringstream ssRegex;
    for (auto i = _tokens.begin(); i != _tokens.end(); ++i) {
        if (i != _tokens.begin())
            ssRegex << "|";
        ssRegex << "(" << *i << ")";
    }

    _regex = ssRegex.str();
}

auto RefStats::match(std::string const& seq, Region const& region) -> Result {
    Result result;
    auto& counts = result.counts;
    auto& ref = result.referenceString;

    int64_t seqlen = _refSeq.seqlen(seq);

    // tokens[0] is the longest token
    int64_t padding = _tokens[0].size() - 1;
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
        counts[whichToken] += overlap;
    }

    result.referenceString = result.referenceString.substr(
        region.begin - paddedRegion.begin,
        region.size());

    return result;
}
