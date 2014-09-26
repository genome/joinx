#include "VcfCompareGtCommand.hpp"

#include "common/Tokenizer.hpp"
#include "common/compat.hpp"
#include "fileformats/VcfReader.hpp"
#include "io/InputStream.hpp"
#include "parse/Kvp.hpp"
#include "processors/MergeSorted.hpp"
#include "reports/VcfCompareGt.hpp"

#include <boost/format.hpp>

#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <unordered_map>

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
}

VcfCompareGtCommand::VcfCompareGtCommand()
{
}

void VcfCompareGtCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<vector<string>>(&filenames_),
            "input file(s) (positional arguments work also)")


        ("output-file,o",
            po::value<std::string>(&outputFile_)->default_value("-"),
            "Path to output report file (- for stdout)")

        ("name,n",
            po::value<vector<string>>(&names_),
            "meaningful names for each of the input files (given in the same order)")

        ("filter-mode,F",
            po::value<vector<std::string>>(&filterTypeStrings_),
            "filtering mode for each input file (filtered, unfiltered, or both. may "
            "be specified more than once)")

        ("sample-name,s",
            po::value<vector<string>>(&sampleNames_),
            "operate only on these samples (applied after sample renaming. "
            "may be specified multiple times)")

        ("result-dir,d",
            po::value<string>(&outputDir_),
            "put variant files here")

        ("sample-rename-file,r",
            po::value<std::string>(&sampleRenameFile_),
            "File containing KEY=VALUE pairs of sample renames "
            "(sample KEY will become VALUE)")

        ("rename-sample,R",
            po::value<std::vector<std::string>>(&sampleRenames_),
            "-R OLD=NEW will rename sample OLD to NEW")
        ;

    _posOpts.add("input-file", -1);
}

boost::unordered_map<std::string, std::string>
VcfCompareGtCommand::sampleRenames() {
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

void VcfCompareGtCommand::finalizeOptions() {
    filterTypes_.resize(filenames_.size());
    std::fill(filterTypes_.begin(), filterTypes_.end(), Vcf::eUNFILTERED);

    for (size_t i = 0; i < filterTypeStrings_.size(); ++i)
        filterTypes_[i] = filterTypeFromString(filterTypeStrings_[i]);
}

void VcfCompareGtCommand::exec() {
    auto sampleRenameMap = sampleRenames();

    if (filenames_.size() < 2) {
        throw std::runtime_error("At least two input files are required");
    }

    std::vector<InputStream::ptr> inputStreams = _streams.openForReading(filenames_);
    ostream* out = _streams.get<std::ostream>(outputFile_);

    if (names_.empty()) {
        names_ = filenames_;
    }
    else if (names_.size() != filenames_.size()) {
        throw std::runtime_error(
            "Mismatch between the number of input files and the number "
            "of names (-n) given");
    }

    auto readers = openVcfs(inputStreams);
    std::vector<Vcf::Header const*> headers;
    std::set<std::string> sampleNameSet;
    for (auto i = readers.begin(); i != readers.end(); ++i) {
        if (!sampleRenameMap.empty())
            (*i)->header().renameSamples(sampleRenameMap);

        auto const& names = (*i)->header().sampleNames();
        std::copy(names.begin(), names.end(), std::inserter(sampleNameSet, sampleNameSet.begin()));
        headers.push_back(&(*i)->header());
    }
    if (sampleNames_.empty()) {
        sampleNames_.resize(sampleNameSet.size());
        std::copy(sampleNameSet.begin(), sampleNameSet.end(), sampleNames_.begin());
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

    VcfCompareGt report(names_, sampleNames_, *out, outputDir_);
    std::size_t numReaders = readers.size();
    auto cmp = Vcf::makeGenotypeComparator(sampleNames_, headers, filterTypes_, numReaders, report);
    MergeSorted<VcfReader> merger(std::move(readers));
    auto e = std::make_unique<Vcf::Entry>();
    while (merger.next(*e)) {
        cmp.push(e.release());
        e = std::make_unique<Vcf::Entry>();
    }
    cmp.finalize();
    report.finalize();
}
