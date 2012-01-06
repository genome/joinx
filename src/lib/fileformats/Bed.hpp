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
class BedHeader {
public:
    static BedHeader fromStream(InputStream& in) {
        return BedHeader();
    }
};

std::ostream& operator<<(std::ostream& s, const BedHeader& h);

class Bed {
public:
    typedef BedHeader HeaderType;
    typedef std::vector<std::string> ExtraFieldsType;

    enum Type {
        SNV,
        INDEL
    };

    Bed();
    // for move semantics
    Bed(Bed const& b);
    Bed(Bed&& b);
    Bed(const std::string& chrom, int64_t start, int64_t stop);
    Bed(const std::string& chrom, int64_t start, int64_t stop, const ExtraFieldsType& extraFields);

    static void parseLine(const BedHeader*, std::string& line, Bed& bed, int maxExtraFields = -1);
    void swap(Bed& rhs);

    const std::string& chrom() const;
    int64_t start() const;
    int64_t stop() const;
    int64_t length() const;
    const std::string& toString() const;

    int cmp(const Bed& rhs) const;
    bool operator<(const Bed& rhs) const;
    bool operator==(const Bed& rhs) const;

    bool type() const {
        return (_stop == _start+1) ? SNV : INDEL;
    }

    const ExtraFieldsType& extraFields() const;

protected:
    std::string _chrom;
    int64_t _start;
    int64_t _stop;
    int64_t _length;
    ExtraFieldsType _extraFields;

    mutable std::string _line;
};

inline const std::string& Bed::chrom() const {
    return _chrom;
}

inline int64_t Bed::start() const {
    return _start;
}

inline int64_t Bed::stop() const {
    return _stop;
}

inline int64_t Bed::length() const {
    return _length;
}

inline bool Bed::operator<(const Bed& rhs) const {
    return cmp(rhs) < 0;
}

inline bool Bed::operator==(const Bed& rhs) const {
    return cmp(rhs) == 0;
}

inline const Bed::ExtraFieldsType& Bed::extraFields() const {
    return _extraFields;
}

std::ostream& operator<<(std::ostream& s, const Bed& bed);
