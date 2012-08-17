#include "VariantContig.hpp"
#include "fileformats/Fasta.hpp"
#include <sstream>
#include <algorithm>

using namespace std;
using Vcf::RawVariant;

VariantContig::VariantContig(RawVariant const& rawvariant, Fasta& reference, int flank, std::string const& seqname) {
    int64_t seqlen = reference.seqlen(seqname);
    int64_t preflank_len = rawvariant.pos < flank ? rawvariant.pos - 1 : flank;
    int64_t postflank_start = rawvariant.pos + rawvariant.ref.size();

    _start = rawvariant.pos - preflank_len;
    _stop = rawvariant.pos + rawvariant.ref.size() - 1 + flank;
    if (_stop > seqlen) {
        _stop = seqlen;
    }

    int64_t postflank_len = _stop - postflank_start + 1;
    
    
    _sequence = reference.sequence(seqname, _start, preflank_len); // left flank
    _sequence += rawvariant.alt;
    _sequence += reference.sequence(seqname, postflank_start, postflank_len); // right flank
    
    stringstream cigar;
    
    int64_t matchlen = preflank_len + std::min(rawvariant.ref.size(), rawvariant.alt.size());
    
    if (rawvariant.ref.size() > rawvariant.alt.size()) {
        cigar << matchlen << "M"
            << rawvariant.ref.size() - rawvariant.alt.size() << "D"
            << postflank_len << "M";
    } else if (rawvariant.ref.size() < rawvariant.alt.size()) {
        cigar << matchlen << "M"
            << rawvariant.alt.size() - rawvariant.ref.size() << "I"
            << postflank_len << "M";
    } else {
        cigar << _stop - _start + 1 << "M";
    }
    _cigar = cigar.str();
}

std::string VariantContig::sequence() const {
    return _sequence;
}
std::string VariantContig::cigar() const {
    return _cigar;
}
int64_t VariantContig::start() const {
    return _start;
}
int64_t VariantContig::stop() const { 
    return _stop;
}
