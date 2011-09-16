#pragma once

#include <ostream>
#include <string>

template<typename T>
class OutputWriter {
public:
    explicit OutputWriter(std::ostream& s, const std::string& sep = "\n")
        : _s(s)
        , _sep(sep)
    {
    }

    void operator()(const T& value) {
        _s << value << _sep;
    }

protected:
    std::ostream& _s;
    std::string _sep;
};
