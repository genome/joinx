#pragma once

#include "intconfig.hpp"

#include <iostream>
#include <string>
#include <vector>

class Bed;
class SnpFilterBase;

class SnpStream {
public:
    SnpStream(const std::string& name, std::istream& in);

    const std::string& name() const;
    uint64_t lineNum() const;
    uint64_t snpCount() const;
    void addFilter(SnpFilterBase* s);

    bool eof() const;
    bool nextSnp(Bed& snp);

protected:
    std::string nextLine();
    bool exclude(const Bed& snp);

protected:
    std::string _name;
    std::istream& _in;
    uint64_t _lineNum;
    uint64_t _snpCount;
    std::vector<SnpFilterBase*> _filters;
};

inline const std::string& SnpStream::name() const {
    return _name;
}

inline uint64_t SnpStream::lineNum() const {
    return _lineNum;
}

inline uint64_t SnpStream::snpCount() const {
    return _snpCount;
}
