#pragma once

#include <ostream>
#include <string>

class DefaultPrinter {
public:
    explicit DefaultPrinter(std::ostream& s, const std::string& sep = "\n")
        : _s(s)
        , _sep(sep)
    {
    }

    template<typename T>
    void operator()(const T& value) {
        _s << value << _sep;
    }

protected:
    std::ostream& _s;
    std::string _sep;
};
