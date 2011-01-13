#include "Variant.hpp"

#include "bedutil/Bed.hpp"
#include <boost/tokenizer.hpp>
#include <stdexcept>
#include <sstream>

using namespace std;

string Variant::typeToString(Type t) {
    switch (t) {
    case SNP:
        return "SNP";
        break;

    case DNP:
        return "DNP";
        break;

    case INS:
        return "INS";
        break;

    case DEL:
        return "DEL";
        break;

    default:
        return "INVALID";
        break;
    }
}

Variant::Type Variant::inferType() const {
    if (_end == _start && !reference().null() && !variant().null())
        return SNP;
    else if (_end == _start+1 && !reference().null() && !variant().null())
        return DNP;
    else if (_reference.null())
        return INS;
    else if (_variant.null())
        return DEL;
    else {
        stringstream ss;
        ss << "Could not determine type of variant: ";
        toStream(ss);
        throw runtime_error(ss.str());
    }
}

Variant::Variant() : _type(INVALID) {}

Variant::Variant(const Bed& bed)
    : _chrom(bed.chrom)
    , _start(bed.start)
    , _end(bed.end)
{
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("/", "", boost::keep_empty_tokens);
    tokenizer tokens(bed.refCall, sep);
    tokenizer::iterator iter = tokens.begin();
    if (iter != tokens.end())
        _reference = *iter++;
    else
        _reference = "-";

    if (iter != tokens.end())
        _variant = *iter++;
    else
        _variant= "-";

    _type = inferType();
    // convert from 0 based bed format
    if (_type == INS)
        ++_end;
    else
        ++_start;
}

ostream& Variant::toStream(ostream& s) const {
    s << chrom() << "\t" <<
        start() << "\t" <<
        end() << "\t" <<
        reference().data() << "\t" <<
        variant().data() << "\t" <<
        typeToString(type());

    return s;
}

