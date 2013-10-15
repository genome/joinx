#pragma once

#include <boost/regex.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <sstream>
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

struct RefStats {
    RefStats(std::vector<std::string> const& tokens)
        : _tokens(tokens)
    {
        std::sort(_tokens.begin(), _tokens.end(), &longer<std::string>);
    }

    void match(std::string const& ref) {
        std::stringstream ssRegex;
        for (auto i = _tokens.begin(); i != _tokens.end(); ++i) {
            counts[*i] = 0;

            if (i != _tokens.begin())
                ssRegex << "|";
            ssRegex << "(" << *i << ")";
        }

        boost::regex re(ssRegex.str());
        boost::sregex_token_iterator begin(ref.begin(), ref.end(), re, 0);
        boost::sregex_token_iterator end;

        for (; begin != end; ++begin) {
            ++counts[*begin];
        }
    }

    std::vector<std::string> _tokens;
    std::unordered_map<std::string, size_t> counts;
};
