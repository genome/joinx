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
#include "fileformats/vcf/SampleTag.hpp"
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
    , _consensusRatio(0.0)
    , _samplePriority(Vcf::MergeStrategy::eORDER)
{
}

void VcfMergeCommand::parseArguments(int argc, char** argv) {
    string consensusOpts;
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("input-file,i", po::value< vector<string> >(&_filenames), "input file(s) (positional arguments work also)")
        ("duplicate-samples,D", po::value< vector<string> >(&_dupSampleFilenames),
            "input file(s) specified as file.vcf=tag. each sample in file.vcf will be duplicated with the name <sample_name>-tag")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("merge-strategy-file,M", po::value<string>(&_mergeStrategyFile), "merge strategy file for info fields (see man page for format)")
        ("clear-filters,c", "When set, merged entries will have FILTER data stripped out")
        ("merge-samples,s", "Allow input files with overlapping samples")
        ("sample-priority,P", po::value<string>(&_samplePrioStr), "sample priority (o=Order, u=Unfiltered, f=Filtered)")
        ("require-consensus,R", po::value<string>(&consensusOpts),
            "When merging samples, require a certain ratio of them to agree, filtering sites that fail. "
            "The format is -R ratio,filterName,filterDescription")
        ;
    po::positional_options_description posOpts;
    posOpts.add("input-file", -1);

    po::variables_map vm;
    auto parsedArgs =
        po::command_line_parser(argc, argv)
            .options(opts)
            .positional(posOpts).run();

    po::store(parsedArgs, vm);

    po::notify(vm);

    if (!consensusOpts.empty()) {
        bool failed = true;
        Tokenizer<char> tok(consensusOpts, ',');
        if (tok.extract(_consensusRatio) && tok.extract(_consensusFilter)) {
            tok.remaining(_consensusFilterDesc);
            if (_consensusRatio >= 0 && _consensusRatio <= 1
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

    if (!vm.count("input-file") && !vm.count("duplicate-samples"))
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

    // recover input file ordering from the command line
    set<string> allFilenames;
    copy(_dupSampleFilenames.begin(), _dupSampleFilenames.end(), inserter(allFilenames, allFilenames.begin()));
    copy(_filenames.begin(), _filenames.end(), inserter(allFilenames, allFilenames.begin()));

    // FIXME: find a better way to recover the ordering of samples specified on the command line
    size_t position = 0;
    for (auto i = parsedArgs.options.begin(); i != parsedArgs.options.end(); ++i) {
        if (i->string_key == "duplicate-samples" && i->value.size() == 1) {
            string const& arg = i->value[0];
            string::size_type eq = arg.find_last_of("=");
            if (eq != string::npos) {
                string fn = arg.substr(0, eq);
                _fileOrder.insert(make_pair(fn, position++));
            }
        } else if (i->string_key == "input-file" && i->value.size() == 1) {
            _fileOrder.insert(make_pair(i->value[0], position++));
        }
    }
}

void VcfMergeCommand::exec() {

    map<string,string> dupSampleMap;
    for (auto iter = _dupSampleFilenames.begin(); iter != _dupSampleFilenames.end(); ++iter) {
        string::size_type eq = iter->find_last_of("=");
        if (eq == string::npos) {
            throw runtime_error(str(format(
                "Invalid value for -D option: %1%. expected <filename>.vcf=<sample suffix>"
                ) %*iter));
        }
        string fn = iter->substr(0, eq);
        string suffix = iter->substr(eq+1);
        dupSampleMap[fn] = suffix;
        _filenames.push_back(fn);
    }

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

    Vcf::Header mergedHeader;
    for (size_t i = 0; i < inputStreams.size(); ++i) {
        readers.push_back(ReaderPtr(new ReaderType(extractor, *inputStreams[i])));

        // if sample duplication is enabled for this file
        auto dupIter = dupSampleMap.find(inputStreams[i]->name());
        if (dupIter != dupSampleMap.end()) {
            auto& header = readers.back()->header();
            vector<string> sampleNames = header.sampleNames();
            size_t nSamples = sampleNames.size();
            for (size_t sampleIdx = 0; sampleIdx < nSamples; ++sampleIdx) {
                string const& oldName = sampleNames[sampleIdx];
                string newName = oldName + dupIter->second;
                header.mirrorSample(oldName, newName);
                Vcf::SampleTag const* oldTag = header.sampleTag(oldName);
                if (oldTag) {
                    Vcf::SampleTag newTag(*oldTag);
                    newTag.set("ID", newName);
                    header.addSampleTag(newTag);
                }
            }
        }

        mergedHeader.merge(readers.back()->header(), _mergeSamples);
        readers.back()->header().sourceIndex(_fileOrder[inputStreams[i]->name()]);
    }

    WriterType writer(*out);
    unique_ptr<Vcf::ConsensusFilter> cnsFilt;
    if (_consensusRatio > 0) {
        mergedHeader.addFilter(_consensusFilter, _consensusFilterDesc);
        if (mergedHeader.formatType("FT") == NULL) {
            CustomType FT("FT", CustomType::FIXED_SIZE, 1, CustomType::STRING, "Sample filter status");
            mergedHeader.addFormatType(FT);
        }
        cnsFilt.reset(new Vcf::ConsensusFilter(_consensusRatio, _consensusFilter, &mergedHeader));
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
