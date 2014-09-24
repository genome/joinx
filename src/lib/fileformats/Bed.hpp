#pragma once

#include "common/CoordinateView.hpp"
#include "common/LocusCompare.hpp"
#include "common/cstdint.hpp"

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
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
    typedef LocusCompare<DefaultCoordinateView, StartAndStop> DefaultCompare;

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

    Bed& operator=(Bed const& b);
    Bed& operator=(Bed&& b);

    static void parseLine(const BedHeader*, std::string& line, Bed& bed, int maxExtraFields = -1);
    void swap(Bed& rhs);

    const std::string& chrom() const;
    int64_t start() const;
    int64_t stop() const;
    int64_t length() const;
    const std::string& toString() const;

    void chrom(std::string chrom);
    void start(int64_t start);
    void stop(int64_t stop);

    bool type() const {
        return (_stop == _start+1) ? SNV : INDEL;
    }

    const ExtraFieldsType& extraFields() const;

    template<typename T>
    void setExtra(size_t idx, T const& value, std::string const& filler = ".");

    bool operator==(Bed const& rhs) const {
        return _chrom == rhs._chrom &&
            _start == rhs._start &&
            _stop == rhs._stop &&
            _extraFields == rhs._extraFields
            ;
    }

protected:
    std::string _chrom;
    int64_t _start;
    int64_t _stop;
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
    return stop() - start();
}

inline const Bed::ExtraFieldsType& Bed::extraFields() const {
    return _extraFields;
}

template<typename T>
inline void Bed::setExtra(size_t idx, T const& value, std::string const& filler /* = "."*/) {
    _line.clear();
    if (_extraFields.size() <= idx) {
        _extraFields.resize(idx + 1, ".");
    }
    _extraFields[idx] = boost::lexical_cast<std::string>(value);
}

inline void Bed::chrom(std::string chrom) {
    _chrom = std::move(chrom);
    _line.clear();
}

inline void Bed::start(int64_t start) {
    _start = start;
    _line.clear();
}

inline void Bed::stop(int64_t stop) {
    _stop = stop;
    _line.clear();
}

inline bool containsInsertions(Bed const& b) {
    return b.length() == 0;
}


std::ostream& operator<<(std::ostream& s, const Bed& bed);
