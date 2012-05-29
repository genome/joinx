#pragma once

#include <string>
#include <cassert>

namespace {
    const int ALLELE_A = 1;
    const int ALLELE_C = 2;
    const int ALLELE_G = 4;
    const int ALLELE_T = 8;
}

inline const char* translateIub(char const base) {
    switch (base) {
        case 'A': return "A"; break;
        case 'C': return "C"; break;
        case 'G': return "G"; break;
        case 'T': return "T"; break;
        case 'M': return "AC"; break;
        case 'K': return "GT"; break;
        case 'Y': return "CT"; break;
        case 'R': return "AG"; break;
        case 'W': return "AT"; break;
        case 'S': return "CG"; break;
        case 'D': return "AGT"; break;
        case 'B': return "CGT"; break;
        case 'H': return "ACT"; break;
        case 'V': return "ACG"; break;
        case 'N': return "ACGT"; break;
        default:  return "";
    }
}

inline const char* translateIub(std::string const& base) {
    assert(base.size() == 1);
    return translateIub(base[0]);
}

inline unsigned alleles2bin(char const* alleles, int len = -1) {
    unsigned rv = 0;
    while (*alleles && len != 0) {
        switch(*alleles++) {
            case 'A': rv |= ALLELE_A; break;
            case 'C': rv |= ALLELE_C; break;
            case 'G': rv |= ALLELE_G; break;
            case 'T': rv |= ALLELE_T; break;
            default: break;
        }
        --len;
    }
    return rv;
}

inline char alleles2iub(unsigned binAlleles) {
    if (binAlleles < 1 || binAlleles > 15)
        return 'N';
    const static char* _tbl = "NACMGRSVTWYHKDBN";
    return _tbl[binAlleles];
}

inline char alleles2iub(const std::string& alleles) {
    return alleles2iub(alleles2bin(alleles.c_str()));
}

inline bool iubOverlap(const std::string& baseA, const std::string& baseB) {
    unsigned iubA = alleles2bin(translateIub(baseA));
    unsigned iubB = alleles2bin(translateIub(baseB));
    return (iubA & iubB) != 0u;
}
