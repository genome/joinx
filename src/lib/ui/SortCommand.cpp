#include "SortCommand.hpp"

#include "common/Exceptions.hpp"
#include "fileformats/BedReader.hpp"
#include "fileformats/ChromPosReader.hpp"
#include "fileformats/InferFileType.hpp"
#include "fileformats/InputStream.hpp"
#include "fileformats/OutputWriter.hpp"
#include "fileformats/StreamFactory.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/Header.hpp"
#include "processors/BedDeduplicator.hpp"
#include "processors/Sort.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <stdexcept>

namespace po = boost::program_options;
using boost::assign::list_of;
using boost::format;
using namespace std;

SortCommand::SortCommand()
    : _outputFile("-")
    , _maxInMem(1000000)
    , _mergeOnly(false)
    , _stable(false)
    , _unique(false)
{
}

void SortCommand::configureOptions() {
    static std::vector<std::string> const defaultInputs = list_of(_outputFile);

    _opts.add_options()
        ("merge-only,m",
            po::bool_switch(&_mergeOnly),
            "merge pre-sorted input files (do not sort)")

        ("input-file,i",
            po::value< vector<string> >(&_filenames)->default_value(defaultInputs, "-"),
            "input file(s) (empty or - means stdin, which is the default)")

        ("output-file,o",
            po::value<string>(&_outputFile)->default_value("-"),
            "output file (empty or - means stdout, which is the default)")

        ("max-mem-lines,M",
            po::value<uint64_t>(&_maxInMem)->default_value(_maxInMem),
            "maximum number of lines to hold in memory at once")

        ("stable,s",
            po::bool_switch(&_stable),
            "perform a 'stable' sort (default=false)")

        ("compression,C",
            po::value<string>(&_compressionString)->default_value(""),
            "type of compression to use for tmpfiles, n=none, g=gzip, b=bzip2. default=n")

        ("unique,u",
            po::bool_switch(&_unique),
            "print only unique entries (bed format only)")
        ;

    _posOpts.add("input-file", -1);
}

namespace {
    template<typename T, typename X>
    vector< boost::shared_ptr<T> > setupStreams(
        const vector< boost::shared_ptr<InputStream> >& v, X& extractor)
    {
        vector< boost::shared_ptr<T> > rv;
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

    vector<InputStream::ptr> inputStreams = _streams.openForReading(_filenames);
    FileType type = detectFormat(inputStreams);
    ostream* out = _streams.get<ostream>(_outputFile);
    if (_streams.cinReferences() > 1)
        throw IOError("stdin listed more than once!");

    // this happens when all input files are empty
    if (type == EMPTY)
        return;


    if (type == CHROMPOS) {
        ChromPosOpenerType cpOpener = boost::bind(&openChromPos, _1);
        typedef OutputWriter<ChromPos> WriterType;
        WriterType writer(*out);
        vector<ChromPosReader::ptr> readers;
        for (auto i = inputStreams.begin(); i != inputStreams.end(); ++i)
            readers.push_back(openChromPos(**i));
        ChromPosHeader outputHeader;

        Sort<ChromPosReader, ChromPosOpenerType, WriterType> sorter(
            readers,
            cpOpener,
            writer,
            outputHeader,
            _maxInMem,
            _stable,
            compression
        );
        sorter.execute();
    } else if (type == BED) {
        int extraFields = _unique ? 1 : 0;
        BedOpenerType bedOpener = boost::bind(&openBed, _1, extraFields);
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
        typedef boost::shared_ptr<ReaderType> ReaderPtr;
        vector<ReaderPtr> readers;

        Vcf::Header mergedHeader;
        VcfExtractor extractor = boost::bind(
                &Vcf::Entry::parseLineAndReheader, _1, &mergedHeader, _2, _3);

        for (auto i = inputStreams.begin(); i != inputStreams.end(); ++i) {
            readers.push_back(ReaderPtr(new ReaderType(extractor, **i)));
            mergedHeader.merge(readers.back()->header(), true);
        }

        *out << mergedHeader;

        VcfOpenerType vcfOpener = boost::bind(&openVcf, _1);
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
