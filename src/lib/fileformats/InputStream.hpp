#pragma once

#include <deque>
#include <istream>
#include <memory>
#include <string>

class InputStream {
public:
    typedef std::shared_ptr<InputStream> ptr;

    static ptr create(const std::string& name, std::istream& s) {
        return ptr(new InputStream(name, s));
    }

    InputStream(const std::string& name, std::istream& s);

    void caching(bool value);
    void rewind();
    bool getline(std::string& line);
    bool eof() const;
    bool good() const;

    const std::string& name() const;

protected:
    std::string _name;
    std::istream& _s;
    bool _caching;
    std::deque<std::string> _cache;
    std::deque<std::string>::iterator _cacheIter;
};

inline const std::string& InputStream::name() const {
    return _name;
}

inline bool InputStream::good() const {
    return _s.good();
}
