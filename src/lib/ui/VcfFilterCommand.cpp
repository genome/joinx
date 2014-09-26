#include "VcfFilterCommand.hpp"

#include "fileformats/DefaultPrinter.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "io/InputStream.hpp"

#include <stdexcept>

namespace po = boost::program_options;
using namespace std;

VcfFilterCommand::VcfFilterCommand()
    : _infile("-")
    , _outputFile("-")
    , _minDepth(0)
{
}

void VcfFilterCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<string>(&_infile)->default_value("-"),
            "input file (empty or - means stdin, which is the default)")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (empty or - means stdout, which is the default)")

        ("min-depth,d",
            po::value<uint32_t>(&_minDepth)->default_value(0),
            "minimum depth")
        ;

    _posOpts.add("input-file", -1);
}

void VcfFilterCommand::exec() {
    InputStream::ptr instream(_streams.openForReading(_infile));
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    DefaultPrinter writer(*out);
    auto reader = openStream<Vcf::Entry>(instream);
    Vcf::Entry e;
    *out << reader->header();
    while (reader->next(e)) {
        e.sampleData().removeLowDepthGenotypes(_minDepth);
        if (e.sampleData().samplesWithData())
            writer(e);
    }
}
