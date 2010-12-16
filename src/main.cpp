#include "Bed.hpp"
#include "BedStream.hpp"
#include "ConcordanceQuality.hpp"
#include "NoReferenceFilter.hpp"
#include "SortBed.hpp"
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

void doSort(const string& infile, const string& outfile) {
    ifstream in(infile.c_str());
    if (!in) throw runtime_error("Failed to open input file " + infile);
    BedStream inp(infile, in);
    SortBed sorter(inp, cout, 1000000);
    sorter.exec();
}

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "usage: " << argv[0] << " {sort|intersect} <file_a> <file_b>" << endl;
        return 1;
    }

    if (string(argv[1]) == "sort") {
        try {
            doSort(argv[2], argv[3]); 
        } catch (const exception& e) {
            cerr << "ERROR: " << e.what() << endl;
        }
    }

    if (string(argv[1]) == "intersect") {
        try {
            snvIntersection(argv[2], argv[3]);
        } catch (const exception& e) {
            cerr << "ERROR: " << e.what() << endl;
        }
    }


    return 0;
}
