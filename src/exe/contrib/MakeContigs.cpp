#include "fileformats/Fasta.hpp"
#include "fileformats/Bed.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

const size_t LINE_LEN = 60;

using namespace std;

class Transcript {
public:
    explicit Transcript(Fasta& srcFa, string const& name, int64_t padding)
        : _name(name)
        , _srcFa(srcFa)
        , _padding(padding)
    {
    }

    void addRegion(Bed& bed) {
        _regions.push_back(bed);
    }

    void write(ostream& outFa, ostream& outRm) {
        sort(_regions.begin(), _regions.end());
        if (_regions.empty()) {
            throw runtime_error("Empty regions for tx " + _name);
        }

        stringstream cigar;
        int64_t seqStart = _regions[0].start();
        if (seqStart > _padding)
            seqStart -= _padding;

        string chrom = _regions[0].chrom();
        // don't read past end of chromosome!
        int64_t seqStop = std::min(
            size_t(_regions.back().stop() + _padding),
            _srcFa.seqlen(chrom)
            );
        
        outFa << ">" << _name << " "
            << chrom << ":" << seqStart << "-" << seqStop << "\n";

        size_t bwritten = 0;

        for (auto i = _regions.begin(); i != _regions.end(); ++i) {
            int64_t start = i->start();
            int64_t stop = i->stop();
            bool last = false;

            if (i == _regions.begin() && start > _padding) {
                start = seqStart;
            }
            // no else, could be first and last
            if (i == _regions.end() - 1) {
                stop = seqStop;
                last = true;
            }

            if (i->chrom() != chrom) {
                throw runtime_error("Chromosome mismatch in tx " + _name);
            }

            string seq = _srcFa.sequence(i->chrom(), start, stop-start+1);
            while (bwritten + seq.size() >= LINE_LEN) {
                outFa << seq.substr(0, LINE_LEN - bwritten) << "\n";
                seq = seq.substr(LINE_LEN - bwritten);
                bwritten = 0;
            }
            outFa << seq;
            bwritten += seq.size();

            cigar << stop-start + 1 << "M";
            if (!last) {
                cigar << (i+1)->start() - stop - 1 << "N";
            } else {
                seqStop = stop;
            }
        }
        if (bwritten)
            outFa << "\n";

        outRm << ">" << _name
            << "-" << _regions[0].chrom()
            << "|" << seqStart 
            << "|" << seqStop 
            << "\n";

        outRm << cigar.str() << "\n";
    }

protected:
    string _name;
    string _chrom;
    Fasta& _srcFa;
    int64_t _padding;
    vector<Bed> _regions;
};

int main(int argc, char** argv) {
    if (argc != 5) {
        cerr << "Usage: " << argv[0]
            << " <fasta> <regions> <outprefix> <padding>\n";
        exit(1);
    }


    Fasta fa(argv[1]);
    fstream regions(argv[2]);
    string outprefix(argv[3]);
    int64_t padding = atoi(argv[4]);
    string line;

    cerr << "Padding is " << padding << "\n";

    ofstream outFa(outprefix + ".fa");
    ofstream outRm(outprefix + ".fa.remap");

    unique_ptr<Transcript> t;

    // beds don't have headers!
    Bed::HeaderType hdr;

    string name;
    while (getline(regions, line)) {
        if (line[0] == '#') {
            if (t)
                t->write(outFa, outRm);
            t.reset(new Transcript(fa, line.substr(2), padding));
        } else {
            Bed b;
            Bed::parseLine(&hdr, line, b);
            t->addRegion(b);
        }
    }

    if (t)
        t->write(outFa, outRm);
}
