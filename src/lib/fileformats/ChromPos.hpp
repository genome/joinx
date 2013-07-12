#pragma once

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class InputStream;

// dummy class, BED doesn't have a header
class ChromPosHeader {
public:
    static ChromPosHeader fromStream(InputStream& in) {
        return ChromPosHeader();
    }
};

std::ostream& operator<<(std::ostream& s, const ChromPosHeader& h);

class ChromPos {
public:
    typedef ChromPosHeader HeaderType;

    ChromPos();
    // for move semantics
    ChromPos(ChromPos const& b);
    ChromPos(ChromPos&& b);

    ChromPos& operator=(ChromPos const& b);
    ChromPos& operator=(ChromPos&& b);


    static void parseLine(const ChromPosHeader*, std::string& line, ChromPos& cp);
    void swap(ChromPos& rhs);

    const std::string& chrom() const;
    int64_t start() const;
    int64_t stop() const;
    const std::string& toString() const;

    int cmp(const ChromPos& rhs) const;
    bool operator<(const ChromPos& rhs) const;
    bool operator==(const ChromPos& rhs) const;

protected:
    std::string _chrom;
    int64_t _start;

    mutable std::string _line;
};

inline const std::string& ChromPos::chrom() const {
    return _chrom;
}

inline int64_t ChromPos::start() const {
    return _start;
}

inline int64_t ChromPos::stop() const {
    return _start;
}

inline bool ChromPos::operator<(const ChromPos& rhs) const {
    return cmp(rhs) < 0;
}

inline bool ChromPos::operator==(const ChromPos& rhs) const {
    return cmp(rhs) == 0;
}

std::ostream& operator<<(std::ostream& s, const ChromPos& bed);
