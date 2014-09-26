#include "VcfCompareCommand.hpp"

#include "common/Tokenizer.hpp"
#include "common/compat.hpp"
#include "fileformats/StreamPump.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/MultiWriter.hpp"
#include "io/InputStream.hpp"
#include "parse/Kvp.hpp"
#include "processors/GroupOverlapping.hpp"
#include "processors/MergeSorted.hpp"
#include "processors/VcfGenotypeMatcher.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <vector>
#include <unordered_map>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;
using boost::format;

namespace {
    std::unordered_map<std::string, Vcf::FilterType> const FILTER_STRINGS_{
          {"unfiltered", Vcf::eUNFILTERED}
        , {"filtered", Vcf::eFILTERED}
        , {"both", Vcf::eBOTH}
        };

    Vcf::FilterType filterTypeFromString(std::string const& name) {
        auto iter = FILTER_STRINGS_.find(name);
        if (iter == FILTER_STRINGS_.end()) {
            throw std::runtime_error(str(format(
                "Invalid filtering mode string '%1%'."
                ) % name));
        }
        return iter->second;
    }

    void nullOutput(Vcf::Entry const&) {}
}

VcfCompareCommand::VcfCompareCommand()
{
}

void VcfCompareCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<vector<string>>(&filenames_),
            "input file(s) (positional arguments work also)")


        ("output-file,o",
            po::value<std::string>(&outputFile_)->default_value("-"),
            "Path to output report file (- for stdout)")

        ("name,n",
            po::value<vector<string>>(&streamNames_),
            "meaningful names for each of the input files (given in the same order)")

        ("filter-mode,F",
            po::value<vector<std::string>>(&filterTypeStrings_),
            "Filtering mode for each input file (filtered, unfiltered, or both. may "
            "be specified more than once)")

        ("sample-name,s",
            po::value<vector<string>>(&sampleNames_),
            "Operate only on these samples (applied after sample renaming. "
            "may be specified multiple times)")

        ("result-dir,d",
            po::value<string>(&outputDir_),
            "Put variant files here")

        ("sample-rename-file,r",
            po::value<std::string>(&sampleRenameFile_),
            "File containing KEY=VALUE pairs of sample renames "
            "(sample KEY will become VALUE)")

        ("rename-sample,R",
            po::value<std::vector<std::string>>(&sampleRenames_),
            "-R OLD=NEW will rename sample OLD to NEW")

        ("exact-format,e",
            po::value<std::string>(&exactFormatField_)->default_value("EXSEC"),
            "Exact match format field name")

        ("partial-format,p",
            po::value<std::string>(&partialFormatField_)->default_value("PXSEC"),
            "Partial match format field name")

        ("include-ref,I",
            po::bool_switch(&includeRefAlleles_)->default_value(false),
            "Include reference alleles in counts")
        ;

    _posOpts.add("input-file", -1);
}

boost::unordered_map<std::string, std::string>
VcfCompareCommand::sampleRenames() {
    boost::unordered_map<std::string, std::string> rv;

    if (!sampleRenameFile_.empty()) {
        auto in = _streams.openForReading(sampleRenameFile_);
        parseKvp(*in, rv);
    }

    for (auto i = sampleRenames_.begin(); i != sampleRenames_.end(); ++i) {
        Tokenizer<char> tok(*i, '=');
        std::string oldName;
        std::string newName;
        if (!tok.extract(oldName) || (tok.remaining(newName), newName.empty())) {
            throw std::runtime_error("invalid sample rename key-value pair: " + *i);
        }
        rv[oldName] = newName;
    }

    for (auto i = rv.begin(); i != rv.end(); ++i) {
        std::cerr << "Renaming sample: " << i->first << " -> " << i->second << "\n";
    }

    return rv;
}

void VcfCompareCommand::finalizeOptions() {
    filterTypes_.resize(filenames_.size());
    std::fill(filterTypes_.begin(), filterTypes_.end(), Vcf::eUNFILTERED);

    for (size_t i = 0; i < filterTypeStrings_.size(); ++i)
        filterTypes_[i] = filterTypeFromString(filterTypeStrings_[i]);
}

void VcfCompareCommand::exec() {
    auto sampleRenameMap = sampleRenames();

    if (filenames_.size() < 2) {
        throw std::runtime_error("At least two input files are required");
    }

    std::vector<InputStream::ptr> inputStreams = _streams.openForReading(filenames_);
    ostream* out = _streams.get<std::ostream>(outputFile_);

    if (streamNames_.empty()) {
        streamNames_ = filenames_;
    }
    else if (streamNames_.size() != filenames_.size()) {
        throw std::runtime_error(
            "Mismatch between the number of input files and the number "
            "of names (-n) given");
    }

    auto readers = openStreams<Vcf::Entry>(inputStreams);
    std::vector<Vcf::Header const*> headers;

    Vcf::CustomType exact(exactFormatField_, Vcf::CustomType::FIXED_SIZE,
        filenames_.size(), Vcf::CustomType::INTEGER, "Exact match indicator");

    Vcf::CustomType partial(partialFormatField_, Vcf::CustomType::FIXED_SIZE,
        filenames_.size(), Vcf::CustomType::INTEGER, "Partial match indicator");

    std::set<std::string> sampleIntersection;
    std::set<std::string> sampleNameSet;

    for (auto i = readers.begin(); i != readers.end(); ++i) {
        auto& header = (*i)->header();
        header.addFormatType(exact);
        header.addFormatType(partial);
        if (!sampleRenameMap.empty())
            header.renameSamples(sampleRenameMap);

        auto names = header.sampleNames();
        std::sort(names.begin(), names.end());

        sampleNameSet.insert(names.begin(), names.end());

        if (i == readers.begin()) {
            // first time
            sampleIntersection.insert(names.begin(), names.end());
        }
        else {
            std::set<std::string> tmp;
            std::set_intersection(
                  sampleIntersection.begin()
                , sampleIntersection.end()
                , names.begin()
                , names.end()
                , std::inserter(tmp, tmp.begin())
                );
            sampleIntersection.swap(tmp);
        }
        headers.push_back(&header);
    }

    if (sampleNames_.empty()) {
        if (sampleIntersection.empty()) {
            throw std::runtime_error("No intersecting samples found in input files!");
        }

        sampleNames_.assign(sampleIntersection.begin(), sampleIntersection.end());
        std::cerr << "Found " << sampleIntersection.size() << " (of "
            << sampleNameSet.size() << " total) intersecting samples.\n";
    }
    else {
        for (auto i = sampleNames_.begin(); i != sampleNames_.end(); ++i) {
            if (sampleNameSet.count(*i) == 0) {
                throw std::runtime_error(str(format(
                    "Requested sample '%1%' does not exist in any input vcf file"
                    ) % *i));
            }
        }
    }

    std::vector<std::string> outputFiles;
    std::unique_ptr<Vcf::MultiWriter> entryWriter;
    boost::function<void(Vcf::Entry const&)> entryCb = &nullOutput;
    if (!outputDir_.empty()) {
        for (size_t i = 0; i < streamNames_.size(); ++i) {
            bfs::path name(streamNames_[i]);
            outputFiles.push_back(str(format("%1%/%2%-%3%") % outputDir_ % i % name.leaf().string()));
        }
        entryWriter = std::make_unique<Vcf::MultiWriter>(outputFiles);

        entryCb = boost::bind(&Vcf::MultiWriter::write, *entryWriter, _1);
        std::cerr << "Writing output to " << outputDir_ << "\n";
    }

    auto merger = makeMergeSorted(readers);
    VcfGenotypeMatcher matcher(
          streamNames_
        , sampleNames_
        , exactFormatField_
        , partialFormatField_
        , filterTypes_
        , entryCb
        , includeRefAlleles_
        );

    auto overlap = makeGroupOverlapping<Vcf::Entry>(matcher);
    auto pump = makePointerStreamPump(merger, overlap);
    pump.execute();
    matcher.reportCounts(*out);
}
