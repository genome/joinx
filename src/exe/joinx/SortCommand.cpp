#include "SortCommand.hpp"

#include "bedutil/MergeSorted.hpp"
#include "bedutil/Sort.hpp"
#include "common/intconfig.hpp"
#include "fileformats/BedStream.hpp"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
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
    for (std::vector<InputPtr>::iterator iter = _inputs.begin(); iter != _inputs.end(); ++iter) {
        delete *iter;
        *iter = NULL;
    }
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
        ("stable,s", "perform a 'stable' sort (default=false)");

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

void SortCommand::exec() {
    typedef boost::shared_ptr<ifstream> FilePtr;
    vector<FilePtr> files;
    bool haveCin = false;
    for (vector<string>::const_iterator iter = _filenames.begin(); iter != _filenames.end(); ++iter) {
        if (*iter != "-") {
            FilePtr in(new ifstream(iter->c_str()));
            if (!in || !in->is_open())
                throw runtime_error(str(format("Failed to open input file %1%") %*iter));
            files.push_back(in);
            _inputs.push_back(new BedStream(*iter, *in)); 
        } else if (!haveCin) {
            _inputs.push_back(new BedStream(*iter, cin));
        } else {
            throw runtime_error("- specified as input file multiple times! Abort.");
        }
    }

    ostream* out;
    ofstream outFile;
    if (!_outputFile.empty() && _outputFile != "-") {
        outFile.open(_outputFile.c_str());
        if (!outFile)
            throw runtime_error(str(format("Failed to open output file %1%") %_outputFile));
        out = &outFile;
    } else {
        out = &cout;
    }

    Sort<BedStream, BedStream*> sorter(_inputs, *out, _maxInMem, _stable);
    sorter.execute();
}
