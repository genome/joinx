#include "common/cstdint.hpp"
#include "io/InputStream.hpp"
#include "io/StreamHandler.hpp"
#include "fileformats/vcf/CustomValue.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"
#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/VcfReader.hpp"
#include "fileformats/vcf/AltNormalizer.hpp"
#include "fileformats/Fasta.hpp"

#include <functional>
#include <iostream>
#include <iterator>
#include <memory>

using namespace std;

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Give a vcf and refseq!\n";
        return 1;
    }

    Fasta fa(argv[2]);
    Vcf::AltNormalizer normalizer(fa);
    StreamHandler streamHandler;
    try {
        InputStream::ptr instream(streamHandler.wrap<istream, InputStream>(argv[1]));
        VcfReader::ptr reader = openVcf(*instream);

        Vcf::Entry entry;
        size_t count(0);
        while (reader->next(entry)) {
            for (auto alt = entry.alt().begin(); alt != entry.alt().end(); ++alt) {
                if (alt->size() != entry.ref().size()) {
                    string sb = entry.toString();
                    Vcf::Entry orig(entry);
                    normalizer.normalize(entry);
                    string sa = entry.toString();
                    if (sa != sb) {
                        uint64_t beg = min(entry.pos(), orig.pos()) - 5;
                        uint64_t end = max(entry.pos()+entry.ref().size(), orig.pos()+orig.ref().size());
                        string seq = fa.sequence(entry.chrom(), beg, end-beg+1);
                        for (size_t i = 0; i < entry.alt().size(); ++i) {
                            string va = seq;
                            string vb = seq;
                            va.replace(entry.pos()-beg, entry.ref().size(), entry.alt()[i]);
                            vb.replace(orig.pos()-beg, orig.ref().size(), orig.alt()[i]);
                            if (va != vb) {
                                cout << entry.chrom() << "\t" << beg << "\t" << seq << "\n";
                                cout << "BEFORE: " << sb << "\n";
                                cout << " AFTER: " << sa << "\n";

                                cout << "XBEFORE: " << va << "\n";
                                cout << "X AFTER: " << vb << "\n";
                            } else {
                                if (++count % 10000 == 0) {
                                    cout << "Processed " << count << " indels (at " << entry.chrom() << " " << entry.pos() << ").\n";
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }


    return 0;
}
