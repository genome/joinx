#pragma once

#include "intconfig.hpp"

#include <iostream>
#include <string>
#include <vector>

class Bed;
class BedFilterBase;

class BedStream {
public:
    BedStream(const std::string& name, std::istream& in);

    const std::string& name() const;
    uint64_t lineNum() const;
    uint64_t bedCount() const;
    void addFilter(BedFilterBase* s);

    bool eof() const;
    bool next(Bed& bed);

protected:
    std::string nextLine();
    bool exclude(const Bed& bed);

protected:
    std::string _name;
    std::istream& _in;
    uint64_t _lineNum;
    uint64_t _bedCount;
    std::vector<BedFilterBase*> _filters;
};

inline const std::string& BedStream::name() const {
    return _name;
}

inline uint64_t BedStream::lineNum() const {
    return _lineNum;
}

inline uint64_t BedStream::bedCount() const {
    return _bedCount;
}
