#pragma once

#include <string>
#include <cassert>

inline const char* translateIub(const std::string& base) {
    assert(base.size() == 1);

    switch (base[0]) {
    case 'A':
        return "A";
        break;
    case 'C':
        return "C";
        break;
    case 'G':
        return "G";
        break;
    case 'T':
        return "T";
        break;
    case 'M':
        return "AC";
        break;
    case 'K':
        return "GT";
        break;
    case 'Y':
        return "CT";
        break;
    case 'R':
        return "AG";
        break;
    case 'W':
        return "AT";
        break;
    case 'S':
        return "GC";
        break;
    case 'D':
        return "AGT";
        break;
    case 'B':
        return "CGT";
        break;
    case 'H':
        return "ACT";
        break;
    case 'V':
        return "ACG";
        break;
    case 'N':
        return "ACGT";
        break;
    default:
        return "";
    }
}
