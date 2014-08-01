#pragma once

#include <common/Tokenizer.hpp>

#include <boost/format.hpp>

#include <cstddef>
#include <string>

template<typename Input, typename Attribute>
void parseKvp(Input& in, Attribute& attr, char delim = '=') {
    std::string line;
    std::size_t lineNum = 0;
    while (in.getline(line)) {
        ++lineNum;
        Tokenizer<char> tok(line, delim);
        std::string key;
        std::string value;
        if (!tok.extract(key) || (tok.remaining(value), value.empty())) {
            using boost::format;
            throw std::runtime_error(str(format(
                "Parse error in %1%, line %2%:\n%3% is not a valid key-value pair"
                ) % in.name() % lineNum % line));
        }
        attr[key] = value;
    }
}
