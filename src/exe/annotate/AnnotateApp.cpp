#include "AnnotateApp.hpp"

#include "bedutil/Intersect.hpp"
#include "annotation/IntersectAnnotation.hpp"
#include "common/Variant.hpp"
#include "common/intconfig.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/BedStream.hpp"
#include "fileformats/TranscriptStructure.hpp"
#include "fileformats/TranscriptStructureStream.hpp"

#include <boost/program_options.hpp>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

AnnotateApp::AnnotateApp(int argc, char** argv)
    : _firstOnly(false)
    , _outputBoth(false)
{
    parseArguments(argc, argv);
}

void AnnotateApp::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help", "this message")
        ("file-a,a", po::value<string>(&_fileA), "input file a (required)")
        ("file-b,b", po::value<string>(&_fileB), "input file b (required)")
        ("first-only,f", "notice only the first thing to hit records in b, not the full intersection")
        ("output-both,", "concatenate intersecting lines in output (vs writing out only lines from 'a')");

    po::positional_options_description posOpts;
    posOpts.add("file-a", 1);
    posOpts.add("file-b", 1);

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv)
            .options(opts)
            .positional(posOpts).run(),
        vm
    );
    po::notify(vm);

    if (vm.count("help")) {
        stringstream ss;
        ss << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("file-a")) {
        stringstream ss;
        ss << "no file-a specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("file-b")) {
        stringstream ss;
        ss << "no file-b specified!" << endl << endl << opts;
        throw runtime_error(ss.str());
    }

    if (vm.count("first-only"))
        _firstOnly = true;
    
    if (vm.count("output-both"))
        _outputBoth = true;
}

namespace {
    // TODO: refactor these output functions into a class
    void onHit(const Variant& v, const TranscriptStructure& b) {
        v.toStream(cout) << "\n";
    }

    void onHitBoth(const Variant& v, const TranscriptStructure& b) {
        const static TranscriptStructure::Field outputFields[] = {
            TranscriptStructure::structure_type,
            TranscriptStructure::transcript_gene_name,
            TranscriptStructure::transcript_transcript_name,
            TranscriptStructure::transcript_species,
            TranscriptStructure::transcript_source,
            TranscriptStructure::transcript_version,
            TranscriptStructure::strand,
            TranscriptStructure::transcript_transcript_status,
//            trv_type,
//            c_position,
//            amino_acid_change,
//            ucsc_cons,
//            domain,
//            all_domains,
//            deletion_substructures,
            TranscriptStructure::transcript_transcript_error
        };
        v.toStream(cout) << "\t";
        cout << b.line() << "\n";
        return;

        unsigned numFields = sizeof(outputFields)/sizeof(outputFields[0]); 
        for (unsigned i = 0; i < numFields-1; ++i)
            cout << b.get(outputFields[i]) << "\t";
        cout << b.get(outputFields[numFields-1]) << "\n";
    }

    class Collector {
    public:
        void missA(const Bed& a) {}
        void missB(const TranscriptStructure& b) {}
        bool hit(const Bed& a, const TranscriptStructure& b) {
            cout << a << "\t" << b.line() << "\n";
            return true;
        }
    };
}

void AnnotateApp::exec() {
    ifstream inA(_fileA.c_str());
    if (!inA)
        throw runtime_error("Failed to open input file '" + _fileA + "'");
    ifstream inB(_fileB.c_str());
    if (!inB)
        throw runtime_error("Failed to open input file '" + _fileB + "'");

    BedStream fa(_fileA, inA, 1);
    TranscriptStructureStream fb(_fileB, inB);
    Collector rc;

    boost::function<void(const Variant&, const TranscriptStructure&)> action = onHit;
    if (_outputBoth)
        action = onHitBoth;

    Intersect<BedStream, TranscriptStructureStream, Collector> intersector(fa, fb, rc);
    intersector.execute();

/*
    IntersectAnnotation intersector(inB, action, _firstOnly);
    
    Bed bed;
    uint64_t lineNo = 0;
    while (fa.next(bed) && intersector.intersect(bed)) {
        ++lineNo;
        if (lineNo % 10000 == 0)
            cerr << "Processed " << lineNo << " lines" << endl;
    }
*/
}
