#include "VcfFilterCommand.hpp"

#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using namespace std::placeholders;

CommandBase::ptr VcfFilterCommand::create(int argc, char** argv) {
    std::shared_ptr<VcfFilterCommand> app(new VcfFilterCommand);
    app->parseArguments(argc, argv);
    return app;
}

VcfFilterCommand::VcfFilterCommand()
    : _infile("-")
    , _outputFile("-")
    , _minDepth(0)
{
}

void VcfFilterCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value<string>(&_infile), "input file (empty or - means stdin, which is the default)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("min-depth,d", po::value<uint32_t>(&_minDepth), "minimum depth")
        ;
    po::positional_options_description posOpts;
    posOpts.add("input-file", -1);

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
}

void VcfFilterCommand::exec() {
    InputStream::ptr instream(_streams.wrap<istream, InputStream>(_infile));
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    typedef function<void(const Vcf::Header*, string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> ReaderType;
    typedef shared_ptr<ReaderType> ReaderPtr;
    typedef OutputWriter<Vcf::Entry> WriterType;

    VcfExtractor extractor = bind(&Vcf::Entry::parseLine, _1, _2, _3);
    WriterType writer(*out);
    ReaderType reader(extractor, *instream);
    Vcf::Entry e;
    *out << reader.header();
    while (reader.next(e)) {
        e.sampleData().removeLowDepthGenotypes(_minDepth);
        if (e.sampleData().samplesWithData())
            writer(e);
    }
}
