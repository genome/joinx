#include "SnvConcordanceCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/TypedStream.hpp"
#include "processors/IntersectFull.hpp"
#include "reports/SnvConcordance.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>
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

SnvConcordanceCommand::SnvConcordanceCommand()
    : _outputFile("-")
    , _useDepth(false)
{
}

void SnvConcordanceCommand::configureOptions() {
    _opts.add_options()

        ("file-a,a",
            po::value<string>(&_fileA)->required(),
            "input .bed file a (required, - for stdin)")

        ("file-b,b",
            po::value<string>(&_fileB)->required(),
            "input .bed file b (required, - for stdin)")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (empty or - means stdout, which is the default)")

        ("depth,d",
            po::bool_switch(&_useDepth),
            "use read depth metric in place of quality")
        ("hits", po::value<string>(&_hitsFile), "output intersection to file, discarded by default")
        ("miss-a", po::value<string>(&_missFileA), "output misses in A to file")
        ("miss-b", po::value<string>(&_missFileB), "output misses in B to file");

    _posOpts.add("file-a", 1);
    _posOpts.add("file-b", 1);
}

namespace {
    class Collector {
    public:
        Collector(
                SnvConcordance& concordance,
                ostream* outHit,
                ostream* outMissA,
                ostream* outMissB
                )
            : _concordance(concordance)
            , _outHit(outHit)
            , _outMissA(outMissA)
            , _outMissB(outMissB)
        {
        }

    bool hit(const Bed& a, const Bed& b) {
        if (_outHit)
            *_outHit << a << "\t" << b << endl;

        return _concordance.hit(a, b);
    }

    void missA(const Bed& a) {
        if (_outMissA)
            *_outMissA << a << "\n";;
        _concordance.missA(a);
    }

    void missB(const Bed& b) {
        if (_outMissB)
            *_outMissB << b << "\n";
        _concordance.missB(b);
    }

    // snv concordance always needs to count misses to compute percentages
    bool wantMissA() const {
        return true;
    }

    bool wantMissB() const {
        return true;
    }

    protected:
        SnvConcordance& _concordance;
        ostream* _outHit;
        ostream* _outMissA;
        ostream* _outMissB;
    };
}

void SnvConcordanceCommand::exec() {
    if (_fileA == _fileB) {
        throw runtime_error("Input files have the same name, '" + _fileA + "', not good.");
    }

    InputStream::ptr inA(_streams.wrap<istream, InputStream>(_fileA));
    InputStream::ptr inB(_streams.wrap<istream, InputStream>(_fileB));

    ostream* out = _streams.get<ostream>(_outputFile);
    ostream* outHit(0);
    ostream* outMissA(0);
    ostream* outMissB(0);

    if (!_hitsFile.empty())
        outHit = _streams.get<ostream>(_hitsFile);
    if (!_missFileA.empty())
        outMissA = _streams.get<ostream>(_missFileA);
    if (!_missFileB.empty())
        outMissB = _streams.get<ostream>(_missFileB);

    if (_streams.cinReferences() > 1)
        throw runtime_error("Multiple input streams from stdin specified. Abort.");


    typedef boost::function<void(const BedHeader*, string&, Bed&)> Extractor;
    Extractor extractor = boost::bind(&Bed::parseLine, _1, _2, _3, -1);
    typedef TypedStream<Bed, Extractor> BedReader;
    BedReader fa(extractor, *inA);
    BedReader fb(extractor, *inB);

    SnvConcordance::DepthOrQual depthOrQual = _useDepth ? SnvConcordance::DEPTH : SnvConcordance::QUAL;
    SnvConcordance concordance(depthOrQual);
    Collector c(concordance, outHit, outMissA, outMissB);
    IntersectFull<BedReader,BedReader,Collector> intersector(fa, fb, c);
    intersector.execute();
    concordance.reportText(*out);
}
