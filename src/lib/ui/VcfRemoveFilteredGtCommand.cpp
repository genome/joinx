#include "VcfRemoveFilteredGtCommand.hpp"

#include "io/InputStream.hpp"
#include "fileformats/VcfReader.hpp"

#include <boost/program_options.hpp>

#include <algorithm>
#include <iterator>

namespace po = boost::program_options;

VcfRemoveFilteredGtCommand::VcfRemoveFilteredGtCommand()
    : inputFile_("-")
    , outputFile_("-")
{
}

void VcfRemoveFilteredGtCommand::configureOptions() {
    _opts.add_options()
        ("input-file,i",
            po::value<std::string>(&inputFile_),
            "input vcf file (- for stdin)")

        ("output-file,o",
            po::value<std::string>(&outputFile_),
            "output vcf file (- for stdout)")

        ("whitelist,w",
            po::value<std::vector<std::string>>(&whitelist_),
            "Filters to keep (can be specified multiple times. use if something "
            "other than PASS/. should be retained)")
        ;

    _posOpts.add("input-file", 1);
    _posOpts.add("output-file", 1);
}

void VcfRemoveFilteredGtCommand::exec() {
    // boost::program_options doesn't like to read into std::set directly
    std::set<std::string> whitelistSet;
    std::copy(whitelist_.begin(), whitelist_.end(),
        std::inserter(whitelistSet, whitelistSet.begin()));

    InputStream::ptr inStream = _streams.openForReading(inputFile_);
    std::ostream* out = _streams.get<std::ostream>(outputFile_);

    auto reader = openVcf(*inStream);
    Vcf::Entry entry;
    *out << reader->header();
    while (reader->next(entry)) {
        entry.sampleData().removeFilteredWhitelist(whitelistSet);
        *out << entry << "\n";
    }
}
