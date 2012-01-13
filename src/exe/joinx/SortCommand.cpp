#include "SortCommand.hpp"

#include "fileformats/BedReader.hpp"
#include "fileformats/InferFileType.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/StreamFactory.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "processors/BedDeduplicator.hpp"
#include "processors/Sort.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;
using namespace std::placeholders;

CommandBase::ptr SortCommand::create(int argc, char** argv) {
    std::shared_ptr<SortCommand> app(new SortCommand);
    app->parseArguments(argc, argv);
    return app;
}

SortCommand::SortCommand()
    : _outputFile("-")
    , _maxInMem(1000000)
    , _mergeOnly(false)
    , _stable(false)
    , _unique(false)
{
}

SortCommand::~SortCommand() {
}

void SortCommand::parseArguments(int argc, char** argv) {
    po::options_description opts("Available Options");
    opts.add_options()
        ("help,h", "this message")
        ("merge-only,m", "merge pre-sorted input files (do not sort)")
        ("input-file,i", po::value< vector<string> >(&_filenames), "input file(s) (empty or - means stdin, which is the default)")
        ("output-file,o", po::value<string>(&_outputFile), "output file (empty or - means stdout, which is the default)")
        ("max-mem-lines,M", po::value<uint64_t>(&_maxInMem),
            str(format("maximum number of lines to hold in memory at once (default=%1%)") %_maxInMem).c_str())
        ("stable,s", "perform a 'stable' sort (default=false)")
        ("compression,C", po::value<string>(&_compressionString), "type of compression to use for tmpfiles, n=none, g=gzip, b=bzip2. default=n")
        ("unique,u", "print only unique entries (bed format only)")
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

    if (vm.count("unique"))
        _unique = true;

    if (!vm.count("input-file"))
        _filenames.push_back("-");

    if (vm.count("merge-only"))
        _mergeOnly = true;

    if (vm.count("stable"))
        _stable = true;

}

namespace {
    template<typename T, typename X>
    vector< shared_ptr<T> > setupStreams(const vector< shared_ptr<InputStream> >& v, X& extractor) {
        vector< shared_ptr<T> > rv;
        for (auto i = v.begin(); i != v.end(); ++i) {
            rv.push_back(shared_ptr<T>(new T(extractor, **i)));
        }
        return rv;
    }

    bool isEmpty(const InputStream::ptr& stream) {
        return inferFileType(*stream) == EMPTY;
    }

    FileType detectFormat(vector<InputStream::ptr>& inputStreams) {
        // remove any empty files
        auto iter = remove_if(inputStreams.begin(), inputStreams.end(), isEmpty);
        inputStreams.erase(iter, inputStreams.end());

        if (inputStreams.empty())
            return EMPTY;

        iter = inputStreams.begin();

        FileType type = inferFileType(**iter++);
        if (type == UNKNOWN)
            throw runtime_error(str(format("Unable to infer file type for %1%") %(*inputStreams.begin())->name()));

        for (; iter != inputStreams.end(); ++iter) {
            FileType otherType = inferFileType(**iter);
            if (otherType != type)
                throw runtime_error(str(format("Multiple file formats detected (%1%), abort.") %(*iter)->name()));
        }

        return type;
    }
}

void SortCommand::exec() {
    CompressionType compression = compressionTypeFromString(_compressionString);

    vector<InputStream::ptr> inputStreams = _streamHandler.wrap<istream, InputStream>(_filenames);
    FileType type = detectFormat(inputStreams);
    ostream* out = _streamHandler.get<ostream>(_outputFile);
    if (_streamHandler.cinReferences() > 1)
        throw runtime_error("stdin listed more than once!");

    // this happens when all input files are empty
    if (type == EMPTY)
        return;

    if (type == BED) {
        int extraFields = _unique ? 1 : 0;
        BedOpenerType bedOpener = bind(&openBed, _1, extraFields);
        typedef OutputWriter<Bed> WriterType;
        WriterType writer(*out);
        vector<BedReader::ptr> readers;
        for (auto i = inputStreams.begin(); i != inputStreams.end(); ++i)
            readers.push_back(openBed(**i, extraFields));
        BedHeader outputHeader;

        if (_unique) {
            BedDeduplicator<WriterType> dedup(writer);
            Sort<BedReader, BedOpenerType, BedDeduplicator<WriterType> > sorter(
                readers,
                bedOpener,
                dedup,
                outputHeader,
                _maxInMem,
                _stable,
                compression
            );
            sorter.execute();
        } else {
            Sort<BedReader, BedOpenerType, WriterType> sorter(
                readers,
                bedOpener,
                writer,
                outputHeader,
                _maxInMem,
                _stable,
                compression
            );
            sorter.execute();
        }
    } else if (type == VCF) {
        typedef OutputWriter<Vcf::Entry> WriterType;
        WriterType writer(*out);

        typedef TypedStream<Vcf::Entry, VcfExtractor> ReaderType;
        typedef shared_ptr<ReaderType> ReaderPtr;
        vector<ReaderPtr> readers;

        Vcf::Header mergedHeader;
        VcfExtractor extractor = bind(&Vcf::Entry::parseLineAndReheader, _1, &mergedHeader, _2, _3);

        for (auto i = inputStreams.begin(); i != inputStreams.end(); ++i) {
            readers.push_back(ReaderPtr(new ReaderType(extractor, **i)));
            mergedHeader.merge(readers.back()->header());
        }

        *out << mergedHeader;

        VcfOpenerType vcfOpener = bind(&openVcf, _1);
        Sort<VcfReader, VcfOpenerType, WriterType> sorter(
            readers,
            vcfOpener,
            writer,
            mergedHeader,
            _maxInMem,
            _stable,
            compression
        );
        sorter.execute();
    } else {
        throw runtime_error("Unknown file type!");
    }
}
