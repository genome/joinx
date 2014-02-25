#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/StreamHandler.hpp"

#include <algorithm>
#include <cstdint>

namespace {
    static int64_t const LARGE_INSERTION = 75;

    template<typename T>
    bool isLargeInsertion(T const& x) {
        // This is naive (doesn't treat padding), but will do for our purposes.
        if (x.alt().size() == 0)
            return false;
        int64_t sizeDiff = int64_t(x.alt()[0].size()) - int64_t(x.ref().size());
        return sizeDiff > LARGE_INSERTION;
    }

    Vcf::Entry* choose(Vcf::Entry* a, Vcf::Entry* b) {
        if (a->alt().empty())
            return b;
        if (b->alt().empty())
            return a;

        bool insA = isLargeInsertion(*a);
        bool insB = isLargeInsertion(*b);

        if (insA && !insB) {
            return a;
        }
        else if (insB && !insA) {
            return b;
        }

        size_t sizeA = std::max(a->ref().size(), a->alt()[0].size());
        size_t sizeB = std::max(b->ref().size(), b->alt()[0].size());
        return sizeA >= sizeB ? a : b;
    }
}

class OverlapHater {
public:
    OverlapHater(std::ostream& out, int spacing)
        : output_(out)
        , spacing_(spacing)
        , entry_(0)
    {
    }

    ~OverlapHater() {
        flush();
    }

    void flush() {
        if (entry_) {
            output_ << *entry_ << "\n";
            delete entry_;
            entry_ = 0;
        }
    }

    void observe(Vcf::Entry* entry) {
        if (entry->chrom() != currentSeq_) {
            currentSeq_ = entry->chrom();
            flush();
            replaceEntry(entry);
            return;
        }

        if (entry_) {
            int64_t distance = int64_t(entry->start()) - int64_t(entry_->stop());
            if (distance >= spacing_) {
                flush();
                replaceEntry(entry);
            }
            else {
                auto winner = choose(entry, entry_);
                if (winner != entry_) {
                    replaceEntry(winner);
                }
                else {
                    std::cout << "###DEBUG: FILTERED " << *entry << "\n";
                    std::cout << "###DEBUG: RETAINING " << *entry_ << "\n";
                }
            }
            std::cout << "###DEBUG: GAP: " << distance << "\n";
        }
        else {
            entry_ = entry;
        }
    }

    void replaceEntry(Vcf::Entry* entry) {
        if (entry_ && entry != entry_) {
            std::cout << "###DEBUG: FILTERED " << *entry_ << "\n";
            std::cout << "###DEBUG: REPLACED WITH " << *entry << "\n";
            delete entry_;
        }
        entry_ = entry;
    }


private:
    std::ostream& output_;
    int spacing_;
    std::string currentSeq_;

    Vcf::Entry* entry_;
};

int main(int argc, char** argv) {
    int spacing = 1;

    if (argc != 3) {
        std::cerr << "Give vcf file, min distance between svs!\n";
        return 1;
    }
    spacing = atoi(argv[2]);
    std::cerr << "Minimum spacing between SVs: " << spacing << "\n";

    StreamHandler streams;
    auto in = streams.openForReading(argv[1]);
    auto reader = openVcf(*in);
    std::unique_ptr<Vcf::Entry> entry(new Vcf::Entry);

    std::cout << reader->header();
    OverlapHater hater(std::cout, spacing);
    while (reader->next(*entry)) {
        hater.observe(entry.release());
        entry.reset(new Vcf::Entry);
    }
}
