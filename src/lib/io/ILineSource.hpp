#pragma once

#include <istream>
#include <memory>
#include <string>

class ILineSource {
public:
    typedef std::unique_ptr<ILineSource> ptr;
    virtual ~ILineSource() {}

    virtual operator bool() const = 0;
    virtual bool getline(std::string& line) = 0;
    virtual char peek() = 0;
    virtual bool eof() const = 0;
    virtual bool good() const = 0;
};
