#pragma once

#include <deque>
#include <istream>
#include <string>

class InputStream {
public:
    InputStream(const std::string& name, std::istream& s);

    void caching(bool value);
    bool getline(std::string& line);
    bool eof() const;

protected:
    std::string _name;
    std::istream& _s;
    bool _caching;
    std::deque<std::string> _cache;
};
