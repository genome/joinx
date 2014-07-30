#include "VcfCompareGtCommand.hpp"

#include "fileformats/InputStream.hpp"
#include "fileformats/VcfReader.hpp"
#include "processors/MergeSorted.hpp"
#include "reports/VcfCompareGt.hpp"

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <vector>
#include <unordered_map>

namespace po = boost::program_options;
using boost::format;
using boost::scoped_ptr;
namespace bfs = boost::filesystem;

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

        ("name,n",
            po::value<vector<string>>(&names_),
            "meaningful names for each of the input files (given in the same order)")

        ("filter-mode,F",
            po::value<vector<std::string>>(&filterTypeStrings_),
            "filtering mode for each input file (filtered, unfiltered, or both. may "
            "be specified more than once)")

        ("sample-name,s",
            po::value<vector<string>>(&sampleNames_),
            "operate only on these samples (may specify multiple times)")

        ("result-dir,d",
            po::value<string>(&outputDir_),
            "put variant files here")
        ;

    _posOpts.add("input-file", -1);
}

void VcfCompareGtCommand::finalizeOptions() {
    filterTypes_.resize(filenames_.size());
    std::fill(filterTypes_.begin(), filterTypes_.end(), Vcf::eUNFILTERED);

    for (size_t i = 0; i < filterTypeStrings_.size(); ++i)
        filterTypes_[i] = filterTypeFromString(filterTypeStrings_[i]);
}

void VcfCompareGtCommand::exec() {
    if (filenames_.size() < 2) {
        throw std::runtime_error("At least two input files are required");
    }

    std::vector<InputStream::ptr> inputStreams = _streams.openForReading(filenames_);
    ostream* out = &std::cout;

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
    MergeSorted<Vcf::Entry, VcfReader::ptr> merger(readers);
    auto cmp = Vcf::makeGenotypeComparator(sampleNames_, headers, filterTypes_, readers.size(), report);
    Vcf::Entry* e(new Vcf::Entry);
    while (merger.next(*e)) {
        cmp.push(e);
        e = new Vcf::Entry;
    }
    delete e;
    cmp.finalize();
    report.finalize();
}
