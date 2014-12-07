#pragma once

#include "fileformats/vcf/Entry.hpp"

#include <ostream>
#include <vector>

struct GroupSortingWriter {
    struct SortHelper_ {
        bool operator()(Vcf::Entry const& x, Vcf::Entry const& y) const {
            if (x.start() < y.start())
                return true;

            if (x.start() > y.start())
                return false;

            return x.stop() < y.stop();
        }
    };

    GroupSortingWriter(std::ostream& out)
        : out(out)
    {}

    ~GroupSortingWriter() {
        endGroup();
    }

    void operator()(Vcf::Entry e) {
        if (!entries.empty() && e.chrom() != entries[0].chrom())
            endGroup();

        entries.push_back(std::move(e));
    }

    void endGroup() {
        std::sort(entries.begin(), entries.end(), SortHelper_{});
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            out << *i << "\n";
        }
        entries.clear();
    }

    std::ostream& out;
    std::vector<Vcf::Entry> entries;
};

