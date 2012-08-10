#include "Compare.hpp"
#include "Entry.hpp"
#include "RawVariant.hpp"
#include "common/VariantType.hpp"

#include <iterator>
#include <string>

using namespace std;

BEGIN_NAMESPACE(Vcf)
BEGIN_NAMESPACE(Compare)

AltIntersect::ResultType AltIntersect::operator()(Entry const& a, Entry const& b) const {
    ResultType rv;
    if (a.chrom() != b.chrom())
        return rv;

    vector<RawVariant> rawA = RawVariant::processEntry(a);
    vector<RawVariant> rawB = RawVariant::processEntry(b);

    map<RawVariant, uint32_t> xsec;

    for (auto i = rawA.begin(); i != rawA.end(); ++i) {
        xsec[*i] = distance(rawA.begin(), i);
    }

    for (auto i = rawB.begin(); i != rawB.end(); ++i) {
        auto xiter = xsec.find(*i);
        if (xiter != xsec.end()) {
            size_t idx = distance(rawB.begin(), i);
            rv[xiter->second] = idx;
        }
    }

    return rv;
}

END_NAMESPACE(Compare)
END_NAMESPACE(Vcf)
