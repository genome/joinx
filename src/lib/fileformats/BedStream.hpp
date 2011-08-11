#pragma once

#include "Bed.hpp"
#include "InputStream.hpp"

#include <cstdint>
#include <string>
#include <vector>

class BedFilterBase;

class BedStream {
public:
    typedef Bed ValueType;

    // maxExtraFields == -1 => no limit
    BedStream(InputStream& in, int maxExtraFields = -1);

    void addFilter(BedFilterBase* filter);

    operator bool() const {
        return !eof();
    }

    const std::string& name() const;
    uint64_t lineNum() const;
    uint64_t bedCount() const;

    bool eof() const;
    void checkEof() const; // check and throw
    bool peek(Bed** bed);
    bool next(Bed& bed);

protected:
    std::string nextLine();
    bool exclude(const Bed& bed);

protected:
    std::string _name;
    InputStream& _in;
    int _maxExtraFields;
    uint64_t _lineNum;
    uint64_t _bedCount;
    std::vector<BedFilterBase*> _filters;

    bool _cached;
    bool _cachedRv;
    Bed _cachedBed;
};

BedStream& operator>>(BedStream& s, Bed& bed);

inline const std::string& BedStream::name() const {
    return _in.name();
}

inline uint64_t BedStream::lineNum() const {
    return _lineNum;
}

inline uint64_t BedStream::bedCount() const {
    return _bedCount;
}
