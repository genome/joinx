#include "Compare.hpp"
#include "Entry.hpp"

#include <iterator>
#include <string>

using namespace std;

BEGIN_NAMESPACE(Vcf)
BEGIN_NAMESPACE(Compare)

AltIntersect::ResultType AltIntersect::operator()(Entry const& a, Entry const& b) const {
    ResultType rv;
    if (a.chrom() != b.chrom() || a.pos() != b.pos() || a.ref() != b.ref())
        return rv;

    map<string, uint32_t> xsec;

    for (auto i = a.alt().begin(); i != a.alt().end(); ++i) {
        xsec[*i] = distance(a.alt().begin(), i);
    }

    for (auto i = b.alt().begin(); i != b.alt().end(); ++i) {
        auto xiter = xsec.find(*i);
        if (xiter != xsec.end()) {
            size_t idx = distance(b.alt().begin(), i);
            rv[xiter->second] = idx;
        }
    }

    return rv;
}

END_NAMESPACE(Compare)
END_NAMESPACE(Vcf)
