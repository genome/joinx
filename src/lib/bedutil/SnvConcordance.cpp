#include "SnvConcordance.hpp"

#include "common/Iub.hpp"
#include "common/Variant.hpp"
#include "fileformats/Bed.hpp"

#include <sstream>


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
                rv << "(" << zygosity.alleleCount << " alleles, " << 
                    overlap << " matching reference) ";
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

SnvConcordance::SnvConcordance() {
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

void SnvConcordance::updateResult(const MatchDescription& md, const ResultCounter& rc) {
    string descB = md.descB.toString();
    pair<MapDescToCount::iterator, bool> result = _results[md.descA.category()][md.matchType].insert(
        make_pair(descB, rc));
    if (!result.second)
        result.first->second += rc;
}

bool SnvConcordance::hit(const Bed& a, const Bed& b) {
    MatchDescription m = matchDescription(Variant(a), Variant(b));

    ResultCounter rc;
    rc.total = 1;
    rc.hits = 1;
    rc.depth = 0;
    updateResult(m, rc);
    return true;
}


void SnvConcordance::missA(const Bed& a) {
}

void SnvConcordance::missB(const Bed& a) {
}

void SnvConcordance::report(std::ostream& s) {
    for (MapType::const_iterator i1 = _results.begin(); i1 != _results.end(); ++i1) {
        s << i1->first << endl;
        for (MapMatchTypeToDescCount::const_iterator i2 = i1->second.begin(); i2 != i1->second.end(); ++i2) {
            s << "\t" << matchTypeString(i2->first) << endl;
            for (MapDescToCount::const_iterator i3 = i2->second.begin(); i3 != i2->second.end(); ++i3) {
                s << "\t\t" << i3->first << ": " << i3->second.hits << endl;
            }
        }
    }
}

