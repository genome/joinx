#include "VcfCompareGtCommand.hpp"

#include "fileformats/InputStream.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/GenotypeComparator.hpp"
#include "processors/MergeSorted.hpp"
#include "reports/VcfCompareGt.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <stdexcept>
#include <vector>

namespace po = boost::program_options;
using boost::format;
using boost::scoped_ptr;
using Vcf::CustomType;

VcfCompareGtCommand::VcfCompareGtCommand()
{
}

void VcfCompareGtCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<vector<string>>(&filenames_),
            "input file(s) (positional arguments work also)")

        ("sample-name,s",
            po::value<vector<string>>(&sampleNames_),
            "operate only on these samples (may specify multiple times)")
        ;

    _posOpts.add("input-file", -1);
}

void VcfCompareGtCommand::exec() {
    std::vector<InputStream::ptr> inputStreams = _streams.openForReading(filenames_);
    ostream* out = &std::cout;

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

    for (size_t i = 0; i < filenames_.size(); ++i) {
        auto const& fn = filenames_[i];
        *out << "#file[" << i << "]=" << fn << '\n';
    }

    VcfCompareGt report(sampleNames_, *out);
    MergeSorted<Vcf::Entry, VcfReader::ptr> merger(readers);
    auto cmp = Vcf::makeGenotypeComparator(sampleNames_, headers, readers.size(), report);
    Vcf::Entry* e(new Vcf::Entry);
    while (merger.next(*e)) {
        cmp.push(e);
        e = new Vcf::Entry;
    }
    delete e;
    cmp.finalize();
    report.finalize();
}
