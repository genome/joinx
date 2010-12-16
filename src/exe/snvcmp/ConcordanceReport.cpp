#include "ConcordanceReport.hpp"

#include "bedutil/BedStream.hpp"
#include "bedutil/ConcordanceQuality.hpp"
#include "bedutil/NoReferenceFilter.hpp"
#include "bedutil/SnvIntersector.hpp"
#include "bedutil/TypeFilter.hpp"

#include <fstream>
#include <stdexcept>

using namespace std;

string ConcordanceReport::name() const {
    return "concordance";
}

void ConcordanceReport::exec(const deque<string>& args) {
    if (args.size() < 2)
        throw runtime_error(name() + " expects 2 positional arguments");

    string fileA = args[0];
    string fileB = args[1];

    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);
    ConcordanceQuality qc;

    ifstream inA(fileA.c_str());
    if (!inA) throw runtime_error("Failed to open input file " + fileA);
    ifstream inB(fileB.c_str());
    if (!inB) throw runtime_error("Failed to open input file " + fileB);

    vector<BedFilterBase*> filters;
    filters.push_back(&nref);
    filters.push_back(&snvOnly);
    BedStream fa(fileA, inA, filters);
    BedStream fb(fileB, inB, filters);

    SnvIntersector snvi(fa, fb, qc);
    snvi.exec();
    qc.report(cout); 

    cout << "Total Snvs: " << fa.bedCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;

}
