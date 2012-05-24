#include "GenotypeMerger.hpp"

#include "Entry.hpp"
#include "GenotypeCall.hpp"
#include "Header.hpp"
#include "CustomType.hpp"
#include "CustomValue.hpp"

#include <boost/format.hpp>
#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)

GenotypeMerger::GenotypeMerger(const Header* header, const vector<string>& alleles)
    : _header(header)
    , _alleles(alleles)
{
}

vector<CustomValue> GenotypeMerger::process(
    const std::vector<CustomType const*>& fields,
    const Entry* e,
    uint32_t sampleIdx,
    const std::vector<size_t>& alleleIndices
    ) const
{
    vector<CustomValue> rv;
    rv.reserve(fields.size());
    for (auto i = fields.begin(); i != fields.end(); ++i) {
        const CustomType* type = *i;
        if (!type)
            throw runtime_error(str(format("Unknown genotype field name '%1%'") %*i));

        if (type->id() == "GT") {
            rv.push_back(CustomValue(type, renumberGT(e, sampleIdx, alleleIndices)));
        } else {
            const CustomValue* v = e->sampleData().get(sampleIdx, type->id());
            rv.push_back(v ? *v : CustomValue());
        }
    }

    return rv;
}

void GenotypeMerger::merge(
    bool overridePreviousValues,
    vector<CustomValue>& previousValues,
    const std::vector<CustomType const*>& fields,
    const Entry* e,
    uint32_t sampleIdx,
    const std::vector<size_t>& alleleIndices
    ) const
{
    assert(previousValues.size() == fields.size());
    for (uint32_t i = 0; i < fields.size(); ++i) {
        const CustomType* type = fields[i];
        if (!type)
            throw runtime_error(str(format("Unknown genotype field name '%1%'") %fields[i]));

        if (fields[i]->id() == "GT") {
            assert(i == 0);
            string newGt = renumberGT(e, sampleIdx, alleleIndices);
            if (!previousValues[0].empty() && !overridePreviousValues) {
                if (areGenotypesDisjoint(previousValues[0].toString(), newGt)) {
                    throw DisjointGenotypesError(
                        str(format("Incompatible genotypes while merging sample data: '%1%' and '%2%'")
                        %previousValues[0].toString() %newGt));
                }
            } else {
                previousValues[i] = CustomValue(type, newGt);
            }
        } else {
            const CustomValue* v = e->sampleData().get(sampleIdx, fields[i]->id());
            if (v && !v->empty() && (overridePreviousValues || previousValues[i].empty())) {
                previousValues[i] = *v;
            }
        }
    }
}

string GenotypeMerger::renumberGT(
    const Entry* e,
    uint32_t sampleIdx,
    const std::vector<size_t>& alleleIndices
    ) const
{
    GenotypeCall const& oldGT = e->sampleData().genotype(sampleIdx);
    if (oldGT.empty())
        return ".";

    char delim = oldGT.phased() ? '|' : '/';

    stringstream newGT;
    for (auto i = oldGT.begin(); i != oldGT.end(); ++i) {
        if (i > oldGT.begin())
            newGT << delim;

        // index > 0 => non-ref
        if (*i > 0) {
            // the index in the alt array will be *i - 1, since 0 denotes ref
            assert(*i-1 < alleleIndices.size());
            uint32_t newIdx = alleleIndices[*i-1];
            assert(newIdx <= _alleles.size());
            newGT << newIdx+1;
        } else {
            // this is the ref allele
            newGT << 0;
        }
    }

    return newGT.str();
}

bool GenotypeMerger::areGenotypesDisjoint(const std::string& str1, const std::string& str2) {
    set<uint32_t> values;
    GenotypeCall gt1(str1);
    GenotypeCall gt2(str2);
    copy(gt1.begin(), gt1.end(), inserter(values, values.begin()));

    for (auto i = gt2.begin(); i != gt2.end(); ++i) {
        if (values.count(*i) > 0)
            return false;
    }

    return true;
}

END_NAMESPACE(Vcf)
