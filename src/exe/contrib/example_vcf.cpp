#include "fileformats/InputStream.hpp"
#include "fileformats/StreamHandler.hpp"
#include "fileformats/TypedStream.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>

using namespace std;
using namespace std::placeholders;

namespace {
uint32_t samplesWithNonRefGenotypes(const Vcf::Entry& entry, const Vcf::Header& h) {
    uint32_t rv(0);
    if (entry.formatDescription().empty() || entry.formatDescription().front() != "GT")
        return 0;

    auto const& sampleData = entry.sampleData();
    // for each sample
    for (auto i = sampleData.begin(); i != sampleData.end(); ++i) {
        if (i->empty())
            continue;

        const Vcf::CustomValue& value = (*i)[0];
        const string* gt = value.get<string>(0);
        if (!gt)
            continue;

        // we don't actually need to parse integers here.
        // since we are looking for anything but '0', it's faster just to look at the string
        string::size_type begin = 0;
        while (begin < gt->size()) {
            string::size_type end = gt->find_first_of("|/", begin);
            if (end == string::npos)
                end = gt->size();

            // if the genotype isn't 1 character long or isn't 0
            // note: this makes 00 not count as 0
            if (end-begin != 1 || (*gt)[begin] != '0') {
                // then we this sample has at least 1 non-ref GT,
                // increment the return value and stop searching this sample
                ++rv;

                // if we wanted the sample name:
                // auto samplePos = distance(sampleData.begin(), i);
                // const string& sampleName = h.sampleNames()[samplePos];
                // cout << sampleName << "\n";
                break;
            }

            begin = end + 1;
        }
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
    InputStream::ptr instream(streamHandler.wrap<istream, InputStream>(argv[1]));

    typedef function<void(const Vcf::Header*, string&, Vcf::Entry&)> VcfExtractor;
    typedef TypedStream<Vcf::Entry, VcfExtractor> VcfReader;

    try {
        Vcf::Header header = Vcf::Header::fromStream(*instream);
        VcfExtractor extractor = bind(&Vcf::Entry::parseLine, _1, _2, _3);
        VcfReader reader(extractor, *instream);
        Vcf::Entry entry;
        while (reader.next(entry)) {
            cout << entry << "\n";
            // how many samples have a non-reference genotype?
            cout << samplesWithNonRefGenotypes(entry, header) << "\n";
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }


    return 0;
}
