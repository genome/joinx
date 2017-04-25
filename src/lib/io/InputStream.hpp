#pragma once

#include "io/ILineSource.hpp"

#include <deque>
#include <istream>
#include <memory>
#include <string>

enum CompressionType {
    NONE,
    GZIP,
    N_COMPRESSION_TYPES
};

CompressionType compressionTypeFromString(const std::string& s);

class InputStream {
public:
    typedef std::unique_ptr<InputStream> ptr;

    static ptr create(const std::string& name, ILineSource::ptr& rawStream);
    static ptr create(const std::string& name, std::istream& rawStream);

    InputStream(const std::string& name, ILineSource::ptr& in);
    InputStream(const std::string& name, std::istream& in);

    void caching(bool value);
    void rewind();
    bool getline(std::string& line);
    bool eof() const;
    bool good() const;
    char peek();
    uint64_t lineNum() const;

    const std::string& name() const;

protected:
    std::string _name;
    std::unique_ptr<ILineSource> _inptr;
    ILineSource& _in;
    bool _caching;
    std::deque<std::string> _cache;
    std::deque<std::string>::iterator _cacheIter;
    uint64_t _lineNum;
};

inline const std::string& InputStream::name() const {
    return _name;
}

inline bool InputStream::good() const {
    return _in.good();
}

inline bool getline(InputStream& s, std::string& line) {
    return bool(s.getline(line));
}
