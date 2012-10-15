#pragma once

#include "Bed.hpp"
#include "InputStream.hpp"

#include <string>

class WiggleReader {
public:
    WiggleReader(InputStream& in, bool stripChr);

    bool next(Bed& value);
    bool eof() const;

protected:
    void newTrack();
    void fixedStep();
    void getEntry(Bed& value);

protected:
    std::string errorMessage(std::string const& msg) const;

protected:
    InputStream& _in;
    bool _stripChr;
    std::string _chrom;
    size_t _posBeg;
    size_t _pos;
    size_t _step;
    size_t _span;

    std::string _line;
    std::string _last;

    size_t _lineNum;
    bool _ready;
};
