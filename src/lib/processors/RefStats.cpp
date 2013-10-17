#include "RefStats.hpp"

#include <boost/regex.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
    template<typename T>
    bool longer(T const& x, T const& y) {
        return x.size() > y.size();
    }
}

RefStats::RefStats(std::vector<std::string> const& toks)
    : _tokens(toks)
{
    std::sort(_tokens.begin(), _tokens.end(), &longer<std::string>);
}

void RefStats::match(std::string const& ref, size_t padding /* = 0*/) {
    std::stringstream ssRegex;
    _counts.clear();
    for (auto i = _tokens.begin(); i != _tokens.end(); ++i) {
        _counts[*i] = 0;

        if (i != _tokens.begin())
            ssRegex << "|";
        ssRegex << "(" << *i << ")";
    }

    boost::regex re(ssRegex.str());
    boost::match_flag_type flags(boost::match_default);
    boost::sregex_token_iterator begin(ref.begin() + padding, ref.end() - padding, re, flags);
    boost::sregex_token_iterator end;

    for (; begin != end; ++begin) {
        auto const& result = *begin;
        _counts[result] += result.length();
    }
}

size_t RefStats::count(std::string const& tok) const {
    auto iter = _counts.find(tok);
    if (iter == _counts.end())
        return 0;
    return iter->second;
}
