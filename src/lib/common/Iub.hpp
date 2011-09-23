#pragma once

#include <string>
#include <cassert>

inline const char* translateIub(const std::string& base) {
    assert(base.size() == 1);

    switch (base[0]) {
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

inline unsigned alleles2bin(const char* alleles) {
    unsigned rv = 0;
    while (*alleles) {
        switch(*alleles++) {
            case 'A': rv |= 1; break;
            case 'C': rv |= 2; break;
            case 'G': rv |= 4; break;
            case 'T': rv |= 8; break;
            default: break;
        }
    }
    return rv;
}

inline char alleles2iub(unsigned binAlleles) {
    char rv = 'N';
    switch (binAlleles) {
        case  1: rv = 'A'; break;
        case  2: rv = 'C'; break;
        case  3: rv = 'M'; break;
        case  4: rv = 'G'; break;
        case  5: rv = 'R'; break;
        case  6: rv = 'S'; break;
        case  7: rv = 'V'; break;
        case  8: rv = 'T'; break;
        case  9: rv = 'W'; break;
        case 10: rv = 'Y'; break;
        case 11: rv = 'H'; break;
        case 12: rv = 'K'; break;
        case 13: rv = 'D'; break;
        case 14: rv = 'B'; break;
        case 15: rv = 'N'; break;
        default: break;
    }
    return rv;
}

inline char alleles2iub(const std::string& alleles) {
    return alleles2iub(alleles2bin(alleles.c_str()));
}

inline bool iubOverlap(const std::string& baseA, const std::string& baseB) {
    unsigned iubA = alleles2bin(translateIub(baseA));
    unsigned iubB = alleles2bin(translateIub(baseB));
    return (iubA & iubB) != 0u;
}
