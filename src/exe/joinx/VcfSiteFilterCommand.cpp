#include "VcfSiteFilterCommand.hpp"

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

CommandBase::ptr VcfSiteFilterCommand::create(int argc, char** argv) {
    std::shared_ptr<VcfSiteFilterCommand> app(new VcfSiteFilterCommand);
    app->parseArguments(argc, argv);
    return app;
}

VcfSiteFilterCommand::VcfSiteFilterCommand()
    : _infile("-")
    , _outputFile("-")
    , _minFailFilter(1.0)
{
}

void VcfSiteFilterCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value<string>(&_infile), "input file (empty or - means stdin, which is the default)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("min-fail-filter,f", po::value<double>(&_minFailFilter), "minimum fraction of failed samples to fail a site")
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

    //construct filter names
    stringstream name;
    name.precision(2);
    name << "sf" << _minFailFilter * 100.0;
    _filterName = name.str();

    stringstream description;
    description.precision(2);
    description << "More than " << _minFailFilter * 100.0 << "% samples with data failed the per-sample filter";
    _filterDescription = description.str();
    
}

void VcfSiteFilterCommand::exec() {
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
    //create filter entry for header
    reader.header().addFilter(_filterName,_filterDescription);
    
    Vcf::Entry e;
    *out << reader.header();
    while (reader.next(e)) {
        uint32_t numSamplesEval = e.sampleData().samplesEvaluatedByFilter();
        int32_t numFailed = e.sampleData().samplesFailedFilter();
        if(numFailed < 0) {
            //there was no FT field available. Warn.
            cerr << "No per-sample filter field available for line " << reader.lineNum() << endl;
        } else {
            if (numSamplesEval && (double) numFailed/ (double) numSamplesEval > _minFailFilter) {
                e.addFilter(_filterName);
            }
            else {
                e.addFilter("PASS");
            }
            writer(e);
        }
    }
}
