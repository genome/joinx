#pragma once

#include "ILineSource.hpp"

class StreamLineSource : public ILineSource {
public:
    explicit StreamLineSource(std::istream& in);

    bool getline(std::string& line);
    char peek();
    bool eof() const;
    bool good() const;
    operator bool() const;

private:
    std::istream& _in;
};

