#include "SnpStream.hpp"
#include "SnpIntersector.hpp"
#include "NoReferenceFilter.hpp"
#include "ConcordanceQuality.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace std;

void snpIntersection(const string& fileA, const string& fileB) {
    NoReferenceFilter nref;
    ConcordanceQuality qc;

    ifstream inA(fileA.c_str());
    if (!inA) throw runtime_error("Failed to open input file " + fileA);
    ifstream inB(fileB.c_str());
    if (!inB) throw runtime_error("Failed to open input file " + fileB);
    SnpStream fa(fileA, inA);
    SnpStream fb(fileB, inB);

    fa.addFilter(&nref);

    SnpIntersector snpi(fa, fb, qc);
    snpi.exec();
    qc.report(cout); 

    cout << "Total Snps: " << fa.snpCount() << endl;
    cout << "      Hits: " << qc.hits() << endl;
    cout << "    Misses: " << qc.misses() << endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "usage: " << argv[0] << " <file_a> <file_b>" << endl;
        return 1;
    }

    try {
        snpIntersection(argv[1], argv[2]);
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
    }


    return 0;
}
