#pragma once

#include <faidx.h>
#include <string>
#include <cstdint>

class FastaReader {
public:
    FastaReader(const std::string& path);
    ~FastaReader();

    const std::string& path() const {
        return _path;
    }

    char sequence(const std::string& region, int32_t pos);

protected:
    faidx_t* _fai;
    std::string _path;
    std::string _region;
    int _len;
    char* _buf;
};
