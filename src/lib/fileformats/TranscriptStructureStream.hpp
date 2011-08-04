#pragma once

#include "TranscriptStructure.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class TranscriptStructureFilterBase;

class TranscriptStructureStream {
public:
    typedef TranscriptStructure ValueType;

    // maxExtraFields == -1 => no limit
    TranscriptStructureStream(const std::string& name, std::istream& in);

    operator bool() const {
        return !eof();
    }

    const std::string& name() const;
    uint64_t lineNum() const;

    bool eof() const;
    void checkEof() const; // check and throw
    bool peek(TranscriptStructure** structure);
    bool next(TranscriptStructure& structure);

protected:
    std::string nextLine();

protected:
    std::string _name;
    std::istream& _in;
    uint64_t _lineNum;
    bool _cached;
    TranscriptStructure _cachedTranscriptStructure;
};

inline const std::string& TranscriptStructureStream::name() const {
    return _name;
}

inline uint64_t TranscriptStructureStream::lineNum() const {
    return _lineNum;
}
