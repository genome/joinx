#include "VariantContig.hpp"
#include "fileformats/Fasta.hpp"
#include <algorithm>

using namespace std;
using Vcf::RawVariant;

VariantContig::VariantContig(
        RawVariant const& var,
        Fasta& ref,
        int flank,
        std::string const& seqname
        )
{
    uint64_t seqlen = ref.seqlen(seqname);
    uint64_t preflank_len = var.pos <= flank ? var.pos - 1 : flank;
    _start = std::max(1ul, var.pos - preflank_len);
    _stop = std::min(var.pos + var.ref.size() - 1 + flank, seqlen);
    uint64_t postflank_start = var.pos + var.ref.size();
    uint64_t postflank_len = _stop - postflank_start + 1;
    
    // build sequence
    if (preflank_len)
        _sequence = ref.sequence(seqname, _start, preflank_len); // left flank
    _sequence += var.alt;
    if (postflank_start <= seqlen && postflank_len)
        _sequence += ref.sequence(seqname, postflank_start, postflank_len); // right flank
    
    // build cigar
    if (preflank_len)
        _cigar.push_back(preflank_len, MATCH);

    if (var.ref.size() > var.alt.size())
        _cigar.push_back(var.ref.size() - var.alt.size(), DEL);
    else if (var.ref.size() < var.alt.size())
        _cigar.push_back(var.alt.size() - var.ref.size(), INS);
    else
        _cigar.push_back(var.alt.size(), MATCH);

    if (postflank_len)
        _cigar.push_back(postflank_len, MATCH);
}
