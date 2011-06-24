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
    // returns the actual end position in case end > length of sequence
    uint32_t sequence(const std::string& chrom, int32_t start, int32_t end, std::string& seq);

protected:
    void loadRegion(const std::string& chrom);

private:
    FastaReader(const FastaReader&);
    FastaReader operator=(const FastaReader&);

protected:
    faidx_t* _fai;
    std::string _path;
    std::string _region;
    int32_t _len;
    char* _buf;
};
