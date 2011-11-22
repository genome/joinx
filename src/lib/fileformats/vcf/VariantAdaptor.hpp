#pragma once

#include "Entry.hpp"
#include "namespace.hpp"

#include <cstdint>

VCF_NAMESPACE_BEGIN

class VariantAdaptor {
public:
    VariantAdaptor(const Entry& entry);

    const Entry& entry() const;

    const std::string& chrom() const;
    int64_t start() const;
    int64_t stop() const;

    std::string toString() const;

    static std::string::size_type commonPrefix(const std::string& a, const std::string& b);

protected:
    Entry _entry;
    int64_t _start;
    int64_t _stop;
};

inline const Entry& VariantAdaptor::entry() const {
    return _entry;
}

inline const std::string& VariantAdaptor::chrom() const {
    return _entry.chrom();
}

inline int64_t VariantAdaptor::start() const {
    return _start;
}

inline int64_t VariantAdaptor::stop() const {
    return _stop;
}

VCF_NAMESPACE_END
