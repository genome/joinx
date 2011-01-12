#include "Variant.hpp"

#include <boost/tokenizer.hpp>
#include <stdexcept>

using namespace std;

Variant::Type Variant::inferType() const {
    if (_bed == NULL)
        throw runtime_error("Variant::inferType called with null BED entry");
    else if (_bed->end == _bed->start+1)
        return SNP;
    else if (_bed->end == _bed->start+2)
        return DNP;
    else if (_reference.data() == "-" || _reference.data() == "0")
        return INS;
    else if (_variant.data() == "-" || _variant.data() == "0")
        return DEL;
    else
        throw runtime_error("Could not determine _variantiant type from _variantiant: " + _bed->line);
}

Variant::Variant() : _bed(NULL), _type(INVALID) {}

Variant::Variant(const Bed* bed) : _bed(bed) {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep("/", "", boost::keep_empty_tokens);
    tokenizer tokens(bed->refCall, sep);
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
}
