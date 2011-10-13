#pragma once

#include <deque>
#include <istream>
#include <memory>
#include <string>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>

#include <boost/iostreams/filtering_stream.hpp>

enum CompressionType {
    NONE,
    GZIP,
    BZIP2,
    N_COMPRESSION_TYPES
};

CompressionType compressionTypeFromString(const std::string& s);

class InputStream {
public:
    typedef std::shared_ptr<InputStream> ptr;

    static ptr create(const std::string& name, std::istream& rawStream);

    InputStream(const std::string& name, std::istream& s);

    void caching(bool value);
    void rewind();
    bool getline(std::string& line);
    bool eof() const;
    bool good() const;
    char peek() const;
    uint64_t lineNum() const;

    const std::string& name() const;

protected:
    std::string _name;
    std::istream& _rawStream;
    boost::iostreams::filtering_stream<boost::iostreams::input> _in;
    boost::iostreams::gzip_decompressor _gzipDecompressor;
    boost::iostreams::bzip2_decompressor _bzip2Decompressor;
    bool _caching;
    std::deque<std::string> _cache;
    std::string _peekBuf;
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
    return s.getline(line);
}
