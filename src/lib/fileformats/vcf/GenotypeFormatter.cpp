#include "GenotypeFormatter.hpp"

#include "Entry.hpp"
#include "Header.hpp"
#include "CustomValue.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <cassert>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

GenotypeFormatter::GenotypeFormatter(const Header* header, const vector<string>& alleles)
    : _header(header)
    , _alleles(alleles)
{
}

vector<CustomValue> GenotypeFormatter::process(
    const std::vector<std::string>& fields,
    const Entry* e,
    uint32_t sampleIdx,
    const std::vector<size_t>& alleleIndices) const
{
    vector<CustomValue> rv;
    rv.reserve(fields.size());
    for (auto i = fields.begin(); i != fields.end(); ++i) {
        const CustomType* type = _header->formatType(*i);
        if (!type)
            throw runtime_error(str(format("Unknown genotype field name '%1%'") %*i));

        if (*i == "GT") {
            rv.push_back(CustomValue(type, renumberGT(e, sampleIdx, alleleIndices)));
        } else {
            const CustomValue* v = e->genotypeData(sampleIdx, *i);
            rv.push_back(v ? *v : CustomValue());
        }
    }

    return rv;
}

string GenotypeFormatter::renumberGT(
        const Entry* e,
        uint32_t sampleIdx,
        const std::vector<size_t>& alleleIndices) const
{
    const CustomValue* oldGT = e->genotypeData(sampleIdx, "GT");
    if (!oldGT)
        return ".";

    stringstream newGT;
    string oldGTstr = oldGT->toString();
    if (oldGTstr.empty())
        return ".";

    Tokenizer<string> t(oldGTstr, "|/");
    uint32_t oldIdx;
    while (t.extract(oldIdx)) {
        if (oldIdx > 0) {
            --oldIdx;
            assert(oldIdx < alleleIndices.size());
            uint32_t newIdx = alleleIndices[oldIdx];
            assert(newIdx < _alleles.size());
            newGT << newIdx+1;
        } else {
            newGT << 0;
        }
        if (t.lastDelim())
            newGT << t.lastDelim();
    }
    return newGT.str();
}

VCF_NAMESPACE_END
