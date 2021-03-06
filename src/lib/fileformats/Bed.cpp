#include "Bed.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <cstring>
#include <iostream>
#include <utility>

using boost::format;
using namespace std;

std::ostream& operator<<(std::ostream& s, const BedHeader& h) {
    return s;
}

Bed::Bed()
    : _start(0)
    , _stop(0)
{}

Bed::Bed(const Bed& b)
    : _chrom(b._chrom)
    , _start(b._start)
    , _stop(b._stop)
    , _extraFields(b._extraFields)
    , _line(b._line)
{
}

Bed::Bed(Bed&& b)
    : _chrom(std::move(b._chrom))
    , _start(b._start)
    , _stop(b._stop)
    , _extraFields(std::move(b._extraFields))
    , _line(std::move(b._line))
{
}

Bed& Bed::operator=(Bed&& b) {
    _chrom = std::move(b._chrom);
    _start = std::move(b._start);
    _stop = std::move(b._stop);
    _extraFields = std::move(b._extraFields);
    _line = std::move(b._line);
    return *this;
}

Bed& Bed::operator=(Bed const& b) {
    _chrom = b._chrom;
    _start = b._start;
    _stop = b._stop;
    _extraFields = b._extraFields;
    _line = b._line;
    return *this;
}

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


void Bed::parseLine(const BedHeader*, std::string& line, Bed& bed, int maxExtraFields) {
    Tokenizer<char> tokenizer(line);
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
        // make ref/call uppercase and translate 0,- meaning "no data" to *
        if (fields == 1) {
            boost::to_upper(extra);
            if (boost::starts_with(extra, "0/") || boost::starts_with(extra, "-/"))
                extra[0] = '*';
            if (boost::ends_with(extra, "/0") || boost::ends_with(extra, "/-"))
                extra[extra.size()-1] = '*';
        }
        bed._extraFields.push_back(std::move(extra));
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

const std::string& Bed::toString() const {
    if (_line.empty()) {
        stringstream ss;
        ss << _chrom << "\t" << _start << "\t" << _stop;
        for (auto iter = _extraFields.begin(); iter != _extraFields.end(); ++iter)
            ss << "\t" << *iter;
        _line = ss.str();
    }
    return _line;
}

std::ostream& operator<<(std::ostream& s, const Bed& bed) {
    s << bed.toString();
    return s;
}
