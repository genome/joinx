#include "Bed.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <cstring>
#include <iostream>

using boost::format;
using namespace std;

Bed::Bed()
    : _start(0)
    , _stop(0)
{}

Bed::Bed(const std::string& chrom, int64_t start, int64_t stop)
    : _chrom(chrom)
    , _start(start)
    , _stop(stop)
{
}

Bed::Bed(const std::string& chrom, int64_t start, int64_t stop, const ExtraFieldsType& extraFields)
    : _chrom(chrom)
    , _start(start)
    , _stop(stop)
    , _extraFields(extraFields)
{
}


void Bed::parseLine(std::string& line, Bed& bed, int maxExtraFields) {
    Tokenizer tokenizer(line);
    if (!tokenizer.extract(bed._chrom))
        throw runtime_error(str(format("Failed to extract chromosome from bed line '%1%'") %line));

    if (!tokenizer.extract(bed._start))
        throw runtime_error(str(format("Failed to extract start position from bed line '%1%'") %line));

    if (!tokenizer.extract(bed._stop))
        throw runtime_error(str(format("Failed to extract stop position from bed line '%1%'") %line));

    bed._extraFields.clear();
    int fields = 0;
    while ((maxExtraFields == -1 || fields++ < maxExtraFields) && !tokenizer.eof()) {
        string extra;
        tokenizer.extract(extra);
        bed._extraFields.push_back(extra);
    }

    bed._line.swap(line);
}

void Bed::swap(Bed& rhs) {
    _chrom.swap(rhs._chrom);
    std::swap(_start, rhs._start);
    std::swap(_stop, rhs._stop);
    _line.swap(rhs._line);
    _extraFields.swap(rhs._extraFields);
}

int Bed::cmp(const Bed& rhs) const {
    int rv = strverscmp(_chrom.c_str(), rhs._chrom.c_str());
    if (rv != 0)
        return rv;

    if (_start < rhs._start)
        return -1;
    if (rhs._start < _start)
        return 1;

    if (_stop < rhs._stop)
        return -1;
    if (rhs._stop < _stop)
        return 1;

    return 0;
}

const std::string& Bed::toString() const {
    if (_line.empty()) {
        stringstream ss;
        ss << _chrom << "\t" << _start << "\t" << _stop;
        for (ExtraFieldsType::const_iterator iter = _extraFields.begin(); iter != _extraFields.end(); ++iter)
            ss << "\t" << *iter;
        _line = ss.str();
    }
    return _line;
}

std::ostream& operator<<(std::ostream& s, const Bed& bed) {
    s << bed.toString();
    return s;
}
