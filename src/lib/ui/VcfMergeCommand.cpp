#include "VcfMergeCommand.hpp"

#include "common/Tokenizer.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/StreamPump.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/AltNormalizer.hpp"
#include "fileformats/vcf/ConsensusFilter.hpp"
#include "fileformats/vcf/CustomType.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/SampleTag.hpp"
#include "io/InputStream.hpp"
#include "processors/Deref.hpp"
#include "processors/MergeSorted.hpp"
#include "processors/VcfEntryMerger.hpp"
#include "processors/VcfFilterer.hpp"
#include "processors/VcfReheaderer.hpp"
#include "processors/VcfSourceIndexDeduplicator.hpp"
#include "processors/grouping/GroupBySharedRegions.hpp"
#include "processors/grouping/GroupForEach.hpp"
#include "processors/grouping/GroupOverlapping.hpp"
#include "processors/grouping/GroupSortingWriter.hpp"
#include "processors/grouping/GroupStats.hpp"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>

#include <memory>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using Vcf::CustomType;

VcfMergeCommand::VcfMergeCommand()
    : _outputFile("-")
    , _clearFilters(false)
    , _mergeSamples(false)
    , _consensusRatio(0.0)
    , _samplePriority(Vcf::MergeStrategy::eORDER)
    , _exactPos(false)
    , _allowSameFile(false)
{
}

void VcfMergeCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value< vector<string> >(&_filenames),
            "input file(s) (positional arguments work also)")

        ("exact-pos,e",
            po::bool_switch(&_exactPos),
            "When set, only entries with exactly the same position are merged "
            "(as opposed to any who overlap)")

        ("duplicate-samples,D",
            po::value< vector<string> >(&_dupSampleFilenames),
            "input file(s) specified as file.vcf=tag. each sample in file.vcf "
            "will be duplicated with the name <sample_name>-tag")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (omit or use '-' for stdout")

        ("merge-strategy-file,M",
            po::value<string>(&_mergeStrategyFile),
            "merge strategy file for info fields (see man page for format)")

        ("clear-filters,c",
            po::bool_switch(&_clearFilters),
            "When set, merged entries will have FILTER data stripped out")

        ("merge-samples,s",
            po::bool_switch(&_mergeSamples),
            "Allow input files with overlapping samples")

        ("normalize-indels,N",
            po::value<string>(&_fastaFile),
            "Normalize indels using the given reference fasta "
            "(may cause output to become unsorted)")

        ("sample-priority,P",
            po::value<string>(&_samplePrioStr),
            "sample priority (o=Order, u=Unfiltered, f=Filtered)")

        ("require-consensus,R",
            po::value<string>(&_consensusOpts),
            "When merging samples, require a certain ratio of them to agree, "
            "filtering sites that fail. "
            "The format is -R ratio,filterName,filterDescription")

        ("allow-same-file",
            po::bool_switch(&_allowSameFile)->default_value(false),
            "Allow merging entries from the same file")

        ("reject-filter-name",
            po::value<string>(&_rejectFilter)->default_value("MERGE_REJECT"),
            "The name of the filter to apply to entries rejected by the merger")

        ("print-stats",
            po::bool_switch(&_printStats)->default_value(false),
            "Print statistics about the size of each bundle of entries being merged")
        ;

    _posOpts.add("input-file", -1);
}

void VcfMergeCommand::finalizeOptions() {
    if (_dupSampleFilenames.empty() && _filenames.empty()) {
        throw runtime_error("No input files specified!");
    }

    // FIXME: make program_options do this for us
    if (!_consensusOpts.empty()) {
        bool failed = true;
        Tokenizer<char> tok(_consensusOpts, ',');
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
                % _consensusOpts));
    }

    // FIXME: make program_options do this for us
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
                ) % _samplePrioStr));
        }
    }

    // recover input file ordering from the command line
    set<string> allFilenames;
    copy(_dupSampleFilenames.begin(), _dupSampleFilenames.end(), inserter(allFilenames, allFilenames.begin()));
    copy(_filenames.begin(), _filenames.end(), inserter(allFilenames, allFilenames.begin()));

    // FIXME: find a better way to recover the ordering of samples specified on the command line
    size_t position = 0;
    for (auto i = _parsedArgs->options.begin(); i != _parsedArgs->options.end(); ++i) {
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

    for (auto iter = _dupSampleFilenames.begin(); iter != _dupSampleFilenames.end(); ++iter) {
        string::size_type eq = iter->find_last_of("=");
        if (eq == string::npos) {
            throw runtime_error(str(format(
                "Invalid value for -D option: %1%. expected <filename>.vcf=<sample suffix>"
                ) %*iter));
        }
        string fn = iter->substr(0, eq);
        string suffix = iter->substr(eq+1);
        _dupSampleMap[fn] = suffix;
        _filenames.push_back(fn);
    }
}

namespace {
    template<typename PrinterType, typename NormalizerType, typename EntryType>
    void writeNormalized(PrinterType& writer, NormalizerType& normalizer, EntryType& entry) {
        normalizer->normalize(entry);
        writer(entry);
    }
}

void VcfMergeCommand::exec() {
    std::unique_ptr<Vcf::AltNormalizer> normalizer;
    std::unique_ptr<Fasta> ref;
    if (!_fastaFile.empty()) {
        ref = std::make_unique<Fasta>(_fastaFile);
        normalizer = std::make_unique<Vcf::AltNormalizer>(*ref);
    }

    vector<InputStream::ptr> inputStreams = _streams.openForReading(_filenames);

    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    auto readers = openStreams<Vcf::Entry>(inputStreams);

    Vcf::Header mergedHeader;
    for (size_t i = 0; i < inputStreams.size(); ++i) {
        // if sample duplication is enabled for this file
        auto dupIter = _dupSampleMap.find(inputStreams[i]->name());
        if (dupIter != _dupSampleMap.end()) {
            auto& header = readers[i]->header();
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

        mergedHeader.merge(readers[i]->header(), _mergeSamples);
        readers[i]->header().sourceIndex(_fileOrder[inputStreams[i]->name()]);
    }

    GroupSortingWriter printer_raw(*out);
    auto printer = std::ref(printer_raw);

    boost::function<void(Vcf::Entry&)> writer;
    if (normalizer) {
        writer = boost::bind(
              &writeNormalized<GroupSortingWriter, std::unique_ptr<Vcf::AltNormalizer>, Vcf::Entry>
            , printer
            , std::ref(normalizer)
            , _1
            );
    }
    else {
        writer = printer;
    }

    std::unique_ptr<Vcf::ConsensusFilter> cnsFilt;
    mergedHeader.addFilter(_rejectFilter, "Rejected by vcf-merge (duplicate locus in same source file)");
    if (_consensusRatio > 0) {
        mergedHeader.addFilter(_consensusFilter, _consensusFilterDesc);
        if (mergedHeader.formatType("FT") == NULL) {
            CustomType FT("FT", CustomType::FIXED_SIZE, 1, CustomType::STRING, "Sample filter status");
            mergedHeader.addFormatType(std::move(FT));
        }

        cnsFilt = std::make_unique<Vcf::ConsensusFilter>(
              _consensusRatio
            , _consensusFilter
            , &mergedHeader
            );
    }

    Vcf::MergeStrategy mergeStrategy(&mergedHeader, _samplePriority, cnsFilt.get());
    mergeStrategy.exactPos(_exactPos);

    if (!_mergeStrategyFile.empty()) {
        InputStream::ptr msFile(_streams.openForReading(_mergeStrategyFile));
        mergeStrategy.parse(*msFile);
    }
    mergeStrategy.clearFilters(_clearFilters);
    mergeStrategy.mergeSamples(_mergeSamples);
    mergeStrategy.primarySampleStreamIndex(0);

    *out << mergedHeader;

    auto entryMerger = makeVcfEntryMerger(writer, &mergedHeader, mergeStrategy);

    // Rejection chain
    auto deref = makeDeref(writer);
    auto splitter = makeGroupForEach(deref);
    auto reheader = makeVcfReheaderer(splitter, &mergedHeader);
    auto filterer = makeVcfFilterer(reheader, _rejectFilter);
    // End rejection chain

    // Dedup will branch between the rejection chain (filterer) and the entryMerger
    auto dedup = makeVcfSourceIndexDeduplicator(entryMerger, filterer, !_allowSameFile);

    auto smallStats = makeGroupStats(dedup, "shared allele bundle size");
    auto regionGrouper = makeGroupBySharedRegions(smallStats);
    auto bigStats = makeGroupStats(regionGrouper, "overlapping bundle size");

    auto initialGrouper = makeGroupOverlapping<Vcf::Entry>(
              bigStats
            , DefaultCoordinateView{}
            , nothing
            , std::bind(&GroupSortingWriter::endGroup, printer)
            );
    auto merger = makeMergeSorted(readers);
    auto pump = makePointerStreamPump(merger, initialGrouper);

    pump.execute();
    initialGrouper.flush();

    if (_printStats) {
        std::cerr << bigStats << smallStats << "\n";
    }
}
