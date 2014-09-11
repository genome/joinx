#include "FindHomopolymersCommand.hpp"

#include "common/Sequence.hpp"
#include "fileformats/Fasta.hpp"
#include "fileformats/Bed.hpp"

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace po = boost::program_options;

namespace {
    // FIXME: if this gets complicated, move it to src/lib/reports
    struct Reporter {
        Reporter(std::string const& sequenceName, std::ostream& out,
            std::array<bool, 256> const& ignore
            )
                : sequenceName(sequenceName)
                , out(out)
                , ignore(ignore)
        {
        }

        void operator()(size_t begin, size_t end, char base) {
            if (!ignore[int(base)]) {
                out << sequenceName << "\t" << begin << "\t" << end << "\t" << base << "\n";
            }
        }

        std::string const& sequenceName;
        std::ostream& out;
        std::array<bool, 256> const& ignore;
    };
}


FindHomopolymersCommand::FindHomopolymersCommand() {
}

void FindHomopolymersCommand::configureOptions() {
    _opts.add_options()
        ("fasta,f",
            po::value<std::string>(&fasta_),
            "input fasta file")

        ("sequences,s",
            po::value<std::vector<std::string>>(&sequences_),
            "optional sequences to restrict to (may be specified multiple times, "
            "default: all sequences)")

        ("min-length,m",
            po::value<size_t>(&minLength_)->default_value(5),
            "minimum homopolymer length")

        ("output-file,o",
            po::value<std::string>(&outputFile_)->default_value("-"),
            "output bed file (- for stdout)")

        ("ignore-chars,I",
            po::value<std::string>(&ignoreChars_)->default_value(""),
            "characters to ignore. prefix with '-' to ignore everything "
            "BUT the given chars (e.g., -I -ACGT)")
        ;

    _posOpts.add("sequences", -1);
}

void FindHomopolymersCommand::finalizeOptions() {
    if (fasta_.empty()) {
        throw std::runtime_error("Required option --fasta missing!");
    }

    bool reverse = !ignoreChars_.empty() && ignoreChars_[0] == '-';
    std::fill(ignoreArray_.begin(), ignoreArray_.end(), reverse);

    for (auto i = ignoreChars_.begin() + (reverse?1:0); i != ignoreChars_.end(); ++i) {
        ignoreArray_[int(*i)] = !reverse;
    }
}

namespace {
    bool strversLessThan(std::string const& x, std::string const& y) {
        return strverscmp(x.c_str(), y.c_str()) < 0;
    }
}

void FindHomopolymersCommand::exec() {
    Fasta fa(fasta_);
    auto faIndex = fa.index();

    std::ostream* out = _streams.get<std::ostream>(outputFile_);

    if (sequences_.empty()) {
        sequences_ = faIndex.sequenceOrder();
        sort(sequences_.begin(), sequences_.end(), strversLessThan);
    }

    for (auto i = sequences_.begin(); i != sequences_.end(); ++i) {
        auto indexEntry = faIndex.entry(*i);
        auto data = fa.sequence(*i, 1, indexEntry->len);
        Reporter reporter(*i, *out, ignoreArray_);
        std::cerr << "Loaded " << data.size() << " bp for sequence " << *i << "\n";
        Sequence::findHomopolymers(data, reporter, minLength_);
    }
}
