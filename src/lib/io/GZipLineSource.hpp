#pragma once

#include "ILineSource.hpp"

#include <zlib.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

class GZipLineSource : public ILineSource {
public:
    class LineBuffer;

    explicit GZipLineSource(int fd);
    explicit GZipLineSource(std::string const& path);
    ~GZipLineSource();

    operator bool() const;
    char peek();
    bool eof() const;
    bool good() const;
    bool getline(std::string& line);

    static size_t bufferSize();

private:
    std::string _path;
    gzFile _fp;
    LineBuffer* _buffer;
    bool _bad;
    bool _eof;
};
