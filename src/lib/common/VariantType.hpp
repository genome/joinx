#pragma once

#include <string>

enum VariantType {
    SUBSTITUTION,
    INSERTION,
    DELETION,
    SV,
    UNKNOWN
};

inline
VariantType variantType(std::string const& ref, std::string const& alt) {
    if (ref.size() < alt.size())
        return INSERTION;
    else if (ref.size() > alt.size())
        return DELETION;
    else if (ref.size() == alt.size())
        return SUBSTITUTION;
    else
        return UNKNOWN;
}

inline
std::string const& variantTypeToString(VariantType type) {
    static std::string const& ins = "INSERTION";
    static std::string const& del = "DELETION";
    static std::string const& sub = "SUBSTITUTION";
    static std::string const& unk = "UNKNOWN";

    switch (type) {
        case INSERTION:     return ins; break;
        case DELETION:      return del; break;
        case SUBSTITUTION:  return sub; break;
        default:            return unk; break;
    }
}
