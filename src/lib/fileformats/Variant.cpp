#include "Variant.hpp"

#include "common/Tokenizer.hpp"
#include "fileformats/Bed.hpp"

#include <boost/format.hpp>
#include <cstdlib>
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
    else
        return INVALID;
}

Variant::Variant() : _type(INVALID) {
    _allSequences.push_back(Sequence("-"));
    _allSequences.push_back(Sequence("-"));
}

Variant::Variant(
    const std::string& chrom,
    int64_t start,
    int64_t stop,
    float quality,
    int32_t depth,
    const std::string& ref,
    const std::string& alt
)   : _chrom(chrom)
    , _start(start)
    , _stop(stop)
    , _quality(quality)
    , _depth(depth)
{
    if (!ref.empty() || ref != "-")
        _allSequences.push_back(ref);
    if (!alt.empty() || alt != "-")
        _allSequences.push_back(alt);
}

Variant::Variant(const Bed& bed)
    : _chrom(bed.chrom())
    , _start(bed.start())
    , _stop (bed.stop())
    , _quality(0.0)
    , _depth(0)
{
    if (!bed.extraFields().empty()) {
        Tokenizer<char> tokenizer(bed.extraFields()[0], '/');
        while (!tokenizer.eof()) {
            string tok;
            tokenizer.extract(tok);
            _allSequences.push_back(tok);
        }
    }
            
    while (_allSequences.size() < 2)
        _allSequences.push_back(Sequence("-"));

    if (bed.extraFields().size() >= 2) {
        if (bed.extraFields()[1] == "-") {
            _quality = 0.0;
        } else {
            const std::string& s = bed.extraFields()[1];
            char* end = NULL;
            _quality = strtod(s.data(), &end);
            if (end - s.data() != ptrdiff_t(s.size()))
                throw runtime_error(str(format("Failed converting quality value '%1%' to number for record '%2%'") %bed.extraFields()[1] %bed.toString()));
        }
    }

    if (bed.extraFields().size() >= 3) {
        stringstream ss(bed.extraFields()[2]);
        if (bed.extraFields()[1] == "-") {
            _depth = 0;
        } else {
            ss >> _depth;
            if (ss.fail())
                throw runtime_error(str(format("Failed converting read depth value '%1%' to number for record '%2%'") %bed.extraFields()[2] %bed.toString()));
        }
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

bool Variant::allelePartialMatch(const Variant& rhs) const {
    if (reference() != rhs.reference())
        return false;

    return iubOverlap(variant().data(), rhs.variant().data());
}

namespace {
    bool doDbSnpMatchHack(const Variant& a, const Variant& dbSnp) {

        // this is nuts
        // we can't trust dbsnp data, so we have to try each allele as the
        // reference, and the combination of all the // rest as the variant.
        // if that doesn't work, we reverse complement and do it again.
        for (unsigned i = 0; i < dbSnp.allSequences().size(); ++i) {

            const Sequence& ref(dbSnp.allSequences()[i]);
            if (ref != a.reference())
                continue;

            unsigned allelesBin = 0;
            for (unsigned j = 0; j < dbSnp.allSequences().size(); ++j) {
                if (j == i) continue; // skip the one we picked as reference
                allelesBin |= alleles2bin(dbSnp.allSequences()[j].data().c_str());
            }
            string iub(1, alleles2iub(allelesBin));
            if (iubOverlap(a.variant().data(), iub))
                return true;
        }
        return false;
    }
}

bool Variant::alleleDbSnpMatch(const Variant& dbSnp) const {

    if (doDbSnpMatchHack(*this, dbSnp))
        return true;

    // didn't match? reverse complement dbsnp and try again;
    Variant revDbSnp(dbSnp);
    revDbSnp.reverseComplement();
    return doDbSnpMatchHack(*this, revDbSnp);
}
