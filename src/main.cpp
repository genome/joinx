#include "Bed.hpp"
#include "BedStream.hpp"
#include "ConcordanceQuality.hpp"
#include "NoReferenceFilter.hpp"
#include "SnvIntersector.hpp"
#include "TypeFilter.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

void snvIntersection(const string& fileA, const string& fileB) {
    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);
    ConcordanceQuality qc;

    ifstream inA(fileA.c_str());
    if (!inA) throw runtime_error("Failed to open input file " + fileA);
    ifstream inB(fileB.c_str());
    if (!inB) throw runtime_error("Failed to open input file " + fileB);
    BedStream fa(fileA, inA);
    BedStream fb(fileB, inB);

    fa.addFilter(&nref);
    fa.addFilter(&snvOnly);

    SnvIntersector snvi(fa, fb, qc);
    snvi.exec();
    qc.report(cout); 

    cout << "Total Snvs: " << fa.bedCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "usage: " << argv[0] << " <file_a> <file_b>" << endl;
        return 1;
    }

    try {
        snvIntersection(argv[1], argv[2]);
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
    }


    return 0;
}
