#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>

class RefStats {
public:
    RefStats(std::vector<std::string> const& toks);

    void match(std::string const& ref, size_t padding = 0);

    size_t count(std::string const& tok) const;

private:
    std::vector<std::string> _tokens;
    std::unordered_map<std::string, size_t> _counts;
};
