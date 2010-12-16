#pragma once

#include "Bed.hpp"
#include "intconfig.hpp"

#include <iostream>
#include <string>
#include <vector>

class BedFilterBase;

class BedStream {
public:
    BedStream(const std::string& name, std::istream& in);
    BedStream(const std::string& name, std::istream& in, BedFilterBase* filter);
    BedStream(const std::string& name, std::istream& in, const std::vector<BedFilterBase*>& filters);

    operator bool() const {
        return _lastGood;
    }

    const std::string& name() const;
    uint64_t lineNum() const;
    uint64_t bedCount() const;

    bool eof() const;
    const Bed& peek() const;
    void advance();

protected:
    std::string nextLine();
    bool exclude(const Bed& bed);

protected:
    std::string _name;
    std::istream& _in;
    uint64_t _lineNum;
    uint64_t _bedCount;
    std::vector<BedFilterBase*> _filters;

    bool _good;
    bool _lastGood;
    Bed _bed;
};

BedStream& operator>>(BedStream& s, Bed& bed);

inline const std::string& BedStream::name() const {
    return _name;
}

inline uint64_t BedStream::lineNum() const {
    return _lineNum;
}

inline uint64_t BedStream::bedCount() const {
    return _bedCount;
}
