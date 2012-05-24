#include "VcfMergeCommand.hpp"

#include "common/Tokenizer.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/Builder.hpp"
#include "fileformats/vcf/ConsensusFilter.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "processors/MergeSorted.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using namespace std::placeholders;
using Vcf::CustomType;

CommandBase::ptr VcfMergeCommand::create(int argc, char** argv) {
    std::shared_ptr<VcfMergeCommand> app(new VcfMergeCommand);
    app->parseArguments(argc, argv);
    return app;
}

VcfMergeCommand::VcfMergeCommand()
    : _outputFile("-")
    , _clearFilters(false)
    , _mergeSamples(false)
    , _consensusPercent(0.0)
    , _samplePriority(Vcf::MergeStrategy::eORDER)
{
}

void VcfMergeCommand::parseArguments(int argc, char** argv) {
    string consensusOpts;
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value< vector<string> >(&_filenames), "input file(s) (positional arguments work also)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("merge-strategy-file,M", po::value<string>(&_mergeStrategyFile), "merge strategy file for info fields (see man page for format)")
        ("clear-filters,c", "When set, merged entries will have FILTER data stripped out")
        ("merge-samples,s", "Allow input files with overlapping samples")
        ("sample-priority,P", po::value<string>(&_samplePrioStr), "sample priority (o=Order, u=Unfiltered, f=Filtered)")
        ("require-consensus,R", po::value<string>(&consensusOpts),
            "When merging samples, require a certain percentage of them to agree, filtering sites that fail. "
            "The format is -R percent,filterName,filterDescription")
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

    if (!consensusOpts.empty()) {
        bool failed = true;
        Tokenizer<char> tok(consensusOpts, ',');
        if (tok.extract(_consensusPercent) && tok.extract(_consensusFilter)) {
            tok.remaining(_consensusFilterDesc);
            if (_consensusPercent >= 0 && _consensusPercent <= 1
                && !_consensusFilter.empty() && !_consensusFilterDesc.empty())
            {
                failed = false;
            }
        }
        if (failed)
            throw runtime_error(str(format(
                "Invalid value for -R: '%1%'. "
                "Expected ratio,filtername,filterdesc with ratio in [0,1], "
                "e.g., '0.5,CNS,Sample consensus filter.'")
                %consensusOpts));
    }

    if (vm.count("help")) {
        stringstream ss;
        ss << opts;
        throw runtime_error(ss.str());
    }

    if (!vm.count("input-file"))
        _filenames.push_back("-");

    if (vm.count("clear-filters"))
        _clearFilters = true;

    if (vm.count("merge-samples"))
        _mergeSamples = true;

    if (!_samplePrioStr.empty()) {
        if (_samplePrioStr == "o") {
            _samplePriority = Vcf::MergeStrategy::eORDER;
        } else if (_samplePrioStr == "u") {
            _samplePriority = Vcf::MergeStrategy::eUNFILTERED;
        } else if (_samplePrioStr == "f") {
            _samplePriority = Vcf::MergeStrategy::eFILTERED;
        } else {
            throw runtime_error(str(format(
                "Unexpected value for --sample-priority/-P: '%1%'.\n"
                "Expected one of o, u, f (for order, unfiltered, filtered)."
                ) %_samplePrioStr));
        }
    }
}

namespace {

}

void VcfMergeCommand::exec() {
    vector<InputStream::ptr> inputStreams = _streams.wrap<istream, InputStream>(_filenames);
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    typedef function<void(const Vcf::Header*, string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> ReaderType;
    typedef shared_ptr<ReaderType> ReaderPtr;
    typedef OutputWriter<Vcf::Entry> WriterType;

    vector<ReaderPtr> readers;
    VcfExtractor extractor = bind(&Vcf::Entry::parseLine, _1, _2, _3);
    uint32_t headerIndex(0);

    Vcf::Header mergedHeader;
    for (size_t i = 0; i < inputStreams.size(); ++i) {
        readers.push_back(ReaderPtr(new ReaderType(extractor, *inputStreams[i])));
        mergedHeader.merge(readers.back()->header(), _mergeSamples);
        readers.back()->header().sourceIndex(headerIndex++);
    }

    WriterType writer(*out);
    unique_ptr<Vcf::ConsensusFilter> cnsFilt;
    if (_consensusPercent > 0) {
        mergedHeader.addFilter(_consensusFilter, _consensusFilterDesc);
        if (mergedHeader.formatType("FT") == NULL) {
            CustomType FT("FT", CustomType::FIXED_SIZE, 1, CustomType::STRING, "Sample filter status");
            mergedHeader.addFormatType(FT);
        }
        cnsFilt.reset(new Vcf::ConsensusFilter(_consensusPercent, _consensusFilter, &mergedHeader));
    }

    Vcf::MergeStrategy mergeStrategy(&mergedHeader, _samplePriority, cnsFilt.get());
    if (!_mergeStrategyFile.empty()) {
        InputStream::ptr msFile(_streams.wrap<istream, InputStream>(_mergeStrategyFile));
        mergeStrategy.parse(*msFile);
    }
    mergeStrategy.clearFilters(_clearFilters);
    mergeStrategy.mergeSamples(_mergeSamples);
    mergeStrategy.primarySampleStreamIndex(0);

    Vcf::Builder builder(mergeStrategy, &mergedHeader, writer);
    *out << mergedHeader;
    MergeSorted<Vcf::Entry, ReaderPtr, Vcf::Builder> merger(readers, builder);
    merger.execute();
    builder.flush();
}
