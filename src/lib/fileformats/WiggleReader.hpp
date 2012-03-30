#pragma once

#include "Bed.hpp"
#include "common/Tokenizer.hpp"

#include <boost/iostreams/device/mapped_file.hpp>
#include <string>

class WiggleReader {
public:
    WiggleReader(std::string const& path, bool stripChr);
    WiggleReader(
        std::string const& name,
        char const* dat,
        size_t len,
        bool stripChr);

    bool next(Bed& value);
    bool eof() const;

protected:
    void newTrack();
    void fixedStep();
    void getEntry(Bed& value);

protected:
    std::string errorMessage(std::string const& msg) const;

protected:
    std::string _path;
    bool _stripChr;
    std::string _chrom;
    size_t _posBeg;
    size_t _pos;
    size_t _step;
    size_t _span;

    JxString _line;
    JxString _last;

    std::unique_ptr<boost::iostreams::mapped_file_source> _f;
    Tokenizer<char> _tok;
    size_t _lineNum;
    bool _ready;
};
