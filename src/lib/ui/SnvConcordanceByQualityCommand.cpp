#include "SnvConcordanceByQualityCommand.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/TypedStream.hpp"
#include "processors/NoReferenceFilter.hpp"
#include "processors/SnvComparator.hpp"
#include "processors/TypeFilter.hpp"
#include "reports/ConcordanceQuality.hpp"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

namespace {
void outputBed(ostream* out, const Bed& b) {
    *out << b << "\n";
}

class ResultCollector {
public:
    typedef boost::function<void(const Bed&)> EventFunc;

    ResultCollector(
        StreamHandler& streams,
        const std::string& hitFileA,
        const std::string& hitFileB,
        const std::string& missFileA,
        const std::string& missFileB)
    {

        if (!hitFileA.empty()) {
            ostream* out = streams.get<ostream>(hitFileA);
            addHitAListener(boost::bind(&outputBed, out, _1));
        }

        if (!hitFileB.empty()) {
            ostream* out = streams.get<ostream>(hitFileB);
            addHitBListener(boost::bind(&outputBed, out, _1));
        }

        if (!missFileA.empty()) {
            ostream* out = streams.get<ostream>(missFileA);
            addMissAListener(boost::bind(&outputBed, out, _1));
        }

        if (!missFileB.empty()) {
            ostream* out = streams.get<ostream>(missFileB);
            addMissBListener(boost::bind(&outputBed, out, _1));
        }
    }

    template<typename T>
    void addAll(T* obj) {
        addHitAListener(boost::bind(&T::hitA, obj, _1));
        addHitBListener(boost::bind(&T::hitB, obj, _1));
        addMissAListener(boost::bind(&T::missA, obj, _1));
        addMissBListener(boost::bind(&T::missB, obj, _1));
    }

    void addHitAListener(const EventFunc& func) { _hitAFuncs.push_back(func); }
    void addHitBListener(const EventFunc& func) { _hitBFuncs.push_back(func); }
    void addMissAListener(const EventFunc& func) { _missAFuncs.push_back(func); }
    void addMissBListener(const EventFunc& func) { _missBFuncs.push_back(func); }

    void hitA(const Bed& x) {
        for (auto i = _hitAFuncs.begin(); i != _hitAFuncs.end(); ++i)
            (*i)(x);
    }

    void hitB(const Bed& x) {
        for (auto i = _hitBFuncs.begin(); i != _hitBFuncs.end(); ++i)
            (*i)(x);
    }

    void missA(const Bed& x) {
        for (auto i = _missAFuncs.begin(); i != _missAFuncs.end(); ++i)
            (*i)(x);
    }

    void missB(const Bed& x) {
        for (auto i = _missBFuncs.begin(); i != _missBFuncs.end(); ++i)
            (*i)(x);
    }

protected:
    vector<EventFunc> _hitAFuncs;
    vector<EventFunc> _hitBFuncs;
    vector<EventFunc> _missAFuncs;
    vector<EventFunc> _missBFuncs;
};
}

SnvConcordanceByQualityCommand::SnvConcordanceByQualityCommand() {
}

void SnvConcordanceByQualityCommand::configureOptions() {
    _opts.add_options()
        ("file-a,a",
            po::value<string>(&_fileA)->required(),
            "input file a (required)")

        ("file-b,b",
            po::value<string>(&_fileB)->required(),
            "input file b (required)")

        ("output-file,o",
            po::value<string>(&_reportFile)->default_value("-"),
            "Report file (defaults to stdout)")

        ("hits-a", po::value<string>(&_hitFileA), "output hits in 'a' to this file")
        ("hits-b",  po::value<string>(&_hitFileB), "output hits in 'b' to this file")
        ("miss-a",  po::value<string>(&_missFileA), "output misses in 'a' to this file")
        ("miss-b",  po::value<string>(&_missFileB), "output misses in 'b' to this file");

    _posOpts.add("file-a", 1);
    _posOpts.add("file-b", 1);
}

void SnvConcordanceByQualityCommand::exec() {

    InputStream::ptr inStreamA(_streams.wrap<istream, InputStream>(_fileA));
    InputStream::ptr inStreamB(_streams.wrap<istream, InputStream>(_fileB));
    ostream* out = _streams.get<ostream>(_reportFile);

    // set up input filters, keep SNV only, and reject entries with N ref value
    NoReferenceFilter nref;
    TypeFilter snvOnly(Bed::SNV);

    auto fa = openBed(*inStreamA, 2);
    auto fb = openBed(*inStreamB, 0);

    fa->addFilter(&snvOnly);
    fa->addFilter(&nref);

    fb->addFilter(&snvOnly);

    ConcordanceQuality qc;
    ResultCollector rc(_streams, _hitFileA, _hitFileB, _missFileA, _missFileB);
    rc.addAll(&qc);

    SnvComparator<BedReader, ResultCollector> snvi(*fa, *fb, rc);
    snvi.exec();
    qc.report(*out);

    *out << "Total Snvs: " << fa->valueCount() << endl;
    *out << "      Hits: " << qc.hits() << endl;
    *out << "    Misses: " << qc.misses() << endl;
}
