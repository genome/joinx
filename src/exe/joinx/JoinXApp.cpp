#include "JoinXApp.hpp"

#include "bedutil/Intersect.hpp"
#include "common/Variant.hpp"
#include "common/intconfig.hpp"
#include "fileformats/Bed.hpp"
#include "fileformats/BedStream.hpp"

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

JoinXApp::JoinXApp(int argc, char** argv)
    : _outputFile("-")
    , _firstOnly(false)
    , _outputBoth(false)
    , _exact(false)
{
    parseArguments(argc, argv);
}

void JoinXApp::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help", "this message")
        ("file-a,a", po::value<string>(&_fileA), "input .bed file a (required, - for stdin)")
        ("file-b,b", po::value<string>(&_fileB), "input .bed file b (required, - for stdin)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("first-only,f", "notice only the first thing to hit records in b, not the full intersection")
        ("output-both", "concatenate intersecting lines in output (vs writing out only lines from 'a')")
        ("exact,e", "require exact match of coordinates (default is to count overlaps)");

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

    if (vm.count("exact"))
        _exact = true;
}

namespace {
    // TODO: make more configurable
    class Collector {
    public:
        Collector(bool outputBoth, bool exact, ostream& s)
            : _outputBoth(outputBoth)
            , _exact(exact)
            , _s(s)
            , _hitCount(0)
        {}

        void hit(const Bed& a, const Bed& b) {
            Variant va(a);
            Variant vb(b);

            // If we are only outputting A or have set 'unique', then skip 
            // things that we just printed.
            // The core intersector returns the full join of A and B.
            // If we are only outputing A, this can look confusing as
            // each time A intersects something in B, an identical line
            // will be printed.
            if (_hitCount > 0 && !_outputBoth) {
                if (_lastA == a)
                    return;
            }

            ++_hitCount;

            if (!_exact || va == vb) {
                _lastA = a;
                _s << a;
                if (_outputBoth)
                    _s << "\t" << b;
                _s << "\n";
            }
        }

    protected:
        Variant _lastA;
        bool _outputBoth;
        bool _exact;
        bool _unique;
        ostream& _s;
        uint32_t _hitCount;
    };
}

void JoinXApp::exec() {
    if (_fileA == _fileB) {
        throw runtime_error("Input files have the same name, '" + _fileA + "', not good.");
    }

    istream* inA(NULL);
    istream* inB(NULL);
    ostream* out(NULL);

    ifstream fA;
    ifstream fB;
    ofstream fOut;

    if (_fileA != "-") {
        fA.open(_fileA.c_str());
        if (!fA)
            throw runtime_error("Failed to open input file '" + _fileA + "'");
        inA = &fA;
    } else {
        inA = &cin;
    }

    if (_fileB != "-") {
        fB.open(_fileB.c_str());
        if (!fB)
            throw runtime_error("Failed to open input file '" + _fileB + "'");
        inB = &fB;
    } else {
        inB = &cin;
    }

    if (!_outputFile.empty() && _outputFile != "-") {
        fOut.open(_outputFile.c_str(), ios::out);
        if (!fOut)
            throw runtime_error("Failed to open output file '" + _outputFile + "'");
        out = &fOut;
    } else {
        out = &cout;
    }
        

    // these bedstreams will read 1 extra field, which is ref/call
    BedStream fa(_fileA, *inA, 1);
    BedStream fb(_fileB, *inB, 1);
    Collector c(_outputBoth, _exact, *out);

    Intersect<BedStream,BedStream,Collector> intersector(fa, fb, c);

    intersector.execute();
}
