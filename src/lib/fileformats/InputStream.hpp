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
    int peek() const;
    uint64_t lineNum() const;

    const std::string& name() const;

protected:
    std::string _name;
    std::istream& _s;
    bool _caching;
    std::deque<std::string> _cache;
    std::deque<std::string>::iterator _cacheIter;
    uint64_t _lineNum;
};

inline const std::string& InputStream::name() const {
    return _name;
}

inline bool InputStream::good() const {
    return _s.good();
}

inline bool getline(InputStream& s, std::string& line) {
    return s.getline(line);
}
