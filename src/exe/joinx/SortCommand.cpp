#include "SortCommand.hpp"

#include "bedutil/MergeSorted.hpp"
#include "bedutil/Sort.hpp"
#include "fileformats/BedStream.hpp"
#include "fileformats/InferFileType.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/vcf/Reader.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <stdexcept>

namespace po = boost::program_options;
using boost::format;
using namespace std;

CommandBase::ptr SortCommand::create(int argc, char** argv) {
    boost::shared_ptr<SortCommand> app(new SortCommand);
    app->parseArguments(argc, argv);
    return app;
}

SortCommand::SortCommand()
    : _outputFile("-")
    , _maxInMem(1000000)
    , _mergeOnly(false)
    , _stable(false)
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
        ("compression,C", po::value<string>(&_compressionString), "type of compression to use for tmpfiles, n=none, g=gzip, z=zlib, b=bzip2. default=n")
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

    if (!vm.count("input-file"))
        _filenames.push_back("-");

    if (vm.count("merge-only"))
        _mergeOnly = true;
    
    if (vm.count("stable"))
        _stable = true;
    
}

namespace {
    template<typename T>
    vector< shared_ptr<T> > setupStreams(const vector< shared_ptr<InputStream> >& v) {
        vector< shared_ptr<T> > rv;
        for (auto i = v.begin(); i != v.end(); ++i) {
            rv.push_back(shared_ptr<T>(new T(**i))); 
        }
        return rv;
    }

    FileType detectFormat(const vector< shared_ptr<InputStream> >& inputStreams) {
        FileType type = inferFileType(**inputStreams.begin());
        if (type == UNKNOWN)
            throw runtime_error(str(format("Unable to infer file type for %1%") %(*inputStreams.begin())->name()));
        for (auto iter = inputStreams.begin()+1; iter != inputStreams.end(); ++iter) {
            if (inferFileType(**iter) != type)
                throw runtime_error(str(format("Multiple file formats detected (%1%), abort.") %(*iter)->name()));
        }

        if (type == VCF && inputStreams.size() > 1) {
            throw runtime_error("VCF only supports sorting one file at a time for now, sorry.");
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

    if (type == BED) {
        vector< shared_ptr<BedStream> > inputs(setupStreams<BedStream>(inputStreams));
        Sort<BedStream, shared_ptr<BedStream> > sorter(inputs, *out, _maxInMem, _stable, compression);
        sorter.execute();
    } else if (type == VCF) {
        vector< shared_ptr<Vcf::Reader> > inputs(setupStreams<Vcf::Reader>(inputStreams));
        *out << inputs[0]->header();
        Sort<Vcf::Reader, shared_ptr<Vcf::Reader> > sorter(inputs, *out, _maxInMem, _stable, compression);
        sorter.execute();
    } else {
        throw runtime_error("Unknown file type!");
    }
}
