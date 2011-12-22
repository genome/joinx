#include "SnvConcordance.hpp"

#include "common/Iub.hpp"
#include "fileformats/Variant.hpp"
#include "fileformats/Bed.hpp"

#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <sstream>

using boost::lexical_cast;
using namespace std;

string SnvDescription::category() const {
    return toString(false);
}

string SnvDescription::toString(bool detail) const {
    stringstream rv;
    switch (zygosity.type) {
        case HETEROZYGOUS:
            rv << "heterozygous ";
            if (detail) {
                unsigned nAlleles = zygosity.alleleCount - overlap;
                rv << "(" << nAlleles << " alleles) ";
            }
            break;

        case HOMOZYGOUS:
            rv << "homozygous ";
            break;

        default:
            rv << "ambiguous zygosity";
            break;
    }


    switch (type) {
        case AMBIGUOUS:
            rv << "ambiguous call";
            break;

        case REFERENCE:
            rv << "reference";
            break;

        case SNV:
            rv << "snv";
            break;

        default:
            rv << "ambiguous call";
            break;
    }

   return rv.str();
}

SnvConcordance::SnvConcordance(DepthOrQual depthOrQual)
    : _depthOrQual(depthOrQual)
{
}

// NOTE: input strings must be sorted lexically (i.e., ACGT)
unsigned SnvConcordance::overlap(const string& sa, const string& sb) {
    unsigned count = 0;
    const char* a = sa.data();
    const char* b = sb.data();

    while (*a != 0 && *b != 0) {
        if (*a < *b)
            ++a;
        else if (*b < *a)
            ++b;
        else {
            ++count;
            ++a; ++b;
        }
    }
    return count;
}

SnvDescription SnvConcordance::describeSnv(const string& refIub, const string& callIub) {
    SnvDescription rv;
    rv.zygosity = zygosity(callIub);
    if (callIub == "N") {
        rv.type = AMBIGUOUS;
    } else if (callIub == refIub) {
        rv.type = REFERENCE;
        rv.overlap = callIub.size();
    } else {
        rv.type = SNV;
        rv.overlap = overlap(refIub, callIub);
    }
    return rv;
}

MatchDescription SnvConcordance::matchDescription(const Variant& a, const Variant& b) {
    string aIub(translateIub(a.variant().data()));
    string bIub(translateIub(b.variant().data()));

    MatchDescription rv;
    if (a.reference() != b.reference()) {
        rv.matchType = REFERENCE_MISMATCH;
    } else if (aIub == bIub) {
        rv.matchType = MATCH;
    } else if (overlap(aIub, bIub) > 0) {
        rv.matchType = PARTIAL_MATCH;
    } else {
        rv.matchType = MISMATCH;
    }

    string refIub(translateIub(a.reference().data()));

    rv.descA = describeSnv(refIub, aIub);
    rv.descB = describeSnv(refIub, bIub);
    return rv;
}

std::string SnvConcordance::matchTypeString(MatchType type) {
    switch (type) {
        case MATCH:
            return "match";
            break;

        case PARTIAL_MATCH:
            return "partial match";
            break;

        case REFERENCE_MISMATCH:
            return "reference mismatch";
            break;

        default:
        case MISMATCH:
            return "mismatch";
            break;
    }
}

void SnvConcordance::incrementTotal(const string& category) {
    pair<map<string,uint64_t>::iterator, bool> tr = _categoryTotals.insert(make_pair(category, 1));
    if (!tr.second)
        ++tr.first->second;
}

void SnvConcordance::updateResult(const MatchDescription& md, const ResultCounter& rc) {
    string category = md.descA.category();
    string descB = md.descB.toString();
    pair<MapDescToCount::iterator, bool> result = _results[category][md.matchType].insert(
        make_pair(descB, rc));
    if (!result.second)
        result.first->second += rc;

    incrementTotal(category);
}

bool SnvConcordance::hit(const Bed& a, const Bed& b) {
    Variant va(a);
    Variant vb(b);
    MatchDescription m = matchDescription(va, vb);

    ResultCounter rc;
    rc.hits = 1;

    if (_depthOrQual == DEPTH)
        rc.depth = vb.depth();
    else
        rc.depth = vb.quality();

    updateResult(m, rc);
    return true;
}


void SnvConcordance::missA(const Bed& a) {
    Variant va(a);
    string refIub(translateIub(va.reference().data()));
    string aIub(translateIub(va.variant().data()));

    SnvDescription descA = describeSnv(refIub, aIub);
    incrementTotal(descA.category());
}

void SnvConcordance::missB(const Bed& a) {
}

void SnvConcordance::reportText(std::ostream& s) {
    for (MapType::const_iterator i1 = _results.begin(); i1 != _results.end(); ++i1) {
        uint64_t total = _categoryTotals[i1->first];
        s << i1->first << "\t" << total << "\n";
        for (MapMatchTypeToDescCount::const_iterator i2 = i1->second.begin(); i2 != i1->second.end(); ++i2) {
            s << "\t" << matchTypeString(i2->first) << "\n";
            for (MapDescToCount::const_iterator i3 = i2->second.begin(); i3 != i2->second.end(); ++i3) {
                double meanDepth = i3->second.depth / double(i3->second.hits);
                s << "\t\t" << i3->first << "\t" << i3->second.hits << "\t" <<
                    fixed << setprecision(2) << meanDepth << "\n";
            }
        }
    }
}
