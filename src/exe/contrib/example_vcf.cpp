#include "fileformats/InputStream.hpp"
#include "fileformats/StreamHandler.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/VcfReader.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>

using namespace std;
using namespace std::placeholders;

namespace {
uint32_t samplesWithNonRefGenotypes(const Vcf::Entry& entry) {
    uint32_t rv(0);
    if (!entry.sampleData().hasGenotypeData())
        return 0;

    for (uint32_t i = 0; i < entry.header().sampleCount(); ++i) {
        Vcf::GenotypeCall gt = entry.sampleData().genotype(i);
        if (gt.empty())
            continue;

        // find the first thing that is not_equal_to 0, i.e., non-ref
        auto iter = find_if(gt.begin(), gt.end(), bind(not_equal_to<uint32_t>(), 0, _1));
        if (iter != gt.end()) // got one? increment!
            ++rv;
    }

    return rv;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Give a filename!\n";
        return 1;
    }

    StreamHandler streamHandler;
    try {
        InputStream::ptr instream(streamHandler.wrap<istream, InputStream>(argv[1]));
        VcfReader::ptr reader = openVcf(*instream);

        Vcf::Entry entry;
        while (reader->next(entry)) {
            cout << entry << "\n";
            // how many samples have a non-reference genotype?
            cout << samplesWithNonRefGenotypes(entry) << "\n";
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }


    return 0;
}
