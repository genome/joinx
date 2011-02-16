#include "Variant.hpp"

#include "fileformats/Bed.hpp"

#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <sstream>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

string Variant::typeToString(Type t) {
    switch (t) {
    case SNP:
        return "SNP";
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
    if (_stop  == _start+1 && !reference().null() && !variant().null())
        return SNP;
    else if (_stop == _start && reference().null() && !variant().null())
        return INS;
    else if (_stop != _start && variant().null())
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
    : _chrom(bed.chrom())
    , _start(bed.start())
    , _stop (bed.stop())
    , _quality(0)
    , _depth(0)
{
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("/", "", boost::keep_empty_tokens);
    tokenizer tokens(bed.extraFields()[0], sep);
    tokenizer::iterator iter = tokens.begin();
    if (iter != tokens.end())
        _reference = *iter++;
    else
        _reference = "-";

    if (iter != tokens.end())
        _variant = *iter++;
    else
        _variant= "-";

    if (bed.extraFields().size() >= 2) {
        stringstream ss(bed.extraFields()[1]);
        ss >> _quality;
        if (ss.fail())
            throw runtime_error(str(format("Failed converting quality value '%1%' to number for record '%2%'") %bed.extraFields()[1] %bed.toString()));
    }

    if (bed.extraFields().size() >= 3) {
        stringstream ss(bed.extraFields()[2]);
        ss >> _depth;
        if (ss.fail())
            throw runtime_error(str(format("Failed converting read depth value '%1%' to number for record '%2%'") %bed.extraFields()[2] %bed.toString()));
    }

    _type = inferType();
}

ostream& Variant::toStream(ostream& s) const {
    s << chrom() << "\t" <<
        start() << "\t" <<
        stop() << "\t" <<
        reference().data() << "\t" <<
        variant().data() << "\t" <<
        typeToString(type());

    return s;
}

