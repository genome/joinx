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
    explicit Transcript(
            Fasta& srcFa, // source fasta (i.e., real referenc e
            string const& name, // sequence name
            int64_t padding // padding to include in base pairs
            )
        : _name(name)
        , _srcFa(srcFa)
        , _padding(padding)
    {
    }

    // add a new region to this transcript
    void addRegion(Bed& bed) {
        _regions.push_back(bed);
    }

    // write out fasta and remap data for this transcript
    void write(ostream& outFa, ostream& outRm) {
        // oops, my file was not sorted properly
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
        
        // write sequence and remap headers
        outFa << ">" << _name << " "
            << chrom << ":" << seqStart << "-" << seqStop << "\n";

        outRm << ">" << _name
            << "-" << chrom << "|" << seqStart << "|" << seqStop << "\n";

        size_t basesThisLine = 0;

        for (auto i = _regions.begin(); i != _regions.end(); ++i) {
            // start and stop for this entry
            int64_t start = i->start() + 1;
            int64_t stop = i->stop();
            // is this the last region in this transcript?
            bool last = (i == _regions.end() - 1); 

            // first region?
            if (i == _regions.begin() && start > _padding)
                start = seqStart; // includes padding

            // last region?
            if (last)
                stop = seqStop; // includes padding

            if (i->chrom() != chrom) // don't do that
                throw runtime_error("Chromosome mismatch in tx " + _name);

            // get the actual bases from the fasta file. Fasta::sequence
            // wants start, length and we have start, stop, so we convert.
            string seq = _srcFa.sequence(i->chrom(), start, stop-start+1);

            // cap line length at 60 for the fasta
            while (basesThisLine + seq.size() >= LINE_LEN) {
                outFa << seq.substr(0, LINE_LEN - basesThisLine) << "\n";
                seq = seq.substr(LINE_LEN - basesThisLine);
                basesThisLine = 0;
            }
            outFa << seq;
            basesThisLine += seq.size();

            // we don't care about line length in the cigar file
            // print out the size of this region as a Match
            outRm << stop-start + 1 << "M";

            // if there are more regions, print out the gap between the end
            // of the current one and the start of the next one as N (which
            // is essentially the same as D).
            if (!last)
                outRm << (i+1)->start() - stop - 1 << "N";
        }

        if (basesThisLine)
            outFa << "\n";
        outRm << "\n";
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

    // set up streams and such
    Fasta fa(argv[1]);
    fstream regions(argv[2]);
    string outprefix(argv[3]);
    int64_t padding = atoi(argv[4]);
    ofstream outFa(outprefix + ".fa");
    ofstream outRm(outprefix + ".fa.remap");

    cerr << "Padding is " << padding << "\n";

    Bed::HeaderType hdr;

    string name;
    string line;
    unique_ptr<Transcript> t;
    while (getline(regions, line)) {
        // begina a new transcript, name = line.substr(2)
        if (line[0] == '#') {
            // if we already had one, flush it out
            if (t)
                t->write(outFa, outRm);
            t.reset(new Transcript(fa, line.substr(2), padding));
        } else {
            Bed b;
            Bed::parseLine(&hdr, line, b);
            t->addRegion(b);
        }
    }

    // flush the last transcript
    if (t)
        t->write(outFa, outRm);
}
