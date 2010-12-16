#pragma once

#include "intconfig.hpp"

#include <iostream>
#include <string>
#include <vector>

class Bed;
class BedFilterBase;

class SnvStream {
public:
    SnvStream(const std::string& name, std::istream& in);

    const std::string& name() const;
    uint64_t lineNum() const;
    uint64_t snvCount() const;
    void addFilter(BedFilterBase* s);

    bool eof() const;
    bool nextSnv(Bed& snv);

protected:
    std::string nextLine();
    bool exclude(const Bed& snv);

protected:
    std::string _name;
    std::istream& _in;
    uint64_t _lineNum;
    uint64_t _snvCount;
    std::vector<BedFilterBase*> _filters;
};

inline const std::string& SnvStream::name() const {
    return _name;
}

inline uint64_t SnvStream::lineNum() const {
    return _lineNum;
}

inline uint64_t SnvStream::snvCount() const {
    return _snvCount;
}
