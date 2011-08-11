#pragma once

#include "Entry.hpp"
#include "fileformats/IGenomicPosition.hpp"
#include "namespace.hpp"

#include <cstdint>

VCF_NAMESPACE_BEGIN

class VariantAdaptor : public IGenomicPosition {
public:
    VariantAdaptor(const Entry& entry);

    const Entry& entry() const;
    uint32_t index() const;
    bool advance();
    void setIndex(uint32_t idx);

    const std::string& chrom() const;
    int64_t start() const;
    int64_t stop() const;
    const std::string& reference() const;
    const std::string& variant() const;

    std::string toString() const;

    static std::string::size_type commonPrefix(const std::string& a, const std::string& b);

protected:
    Entry _entry;
    uint32_t _idx;
    int64_t _start;
    int64_t _stop;
    std::string _ref;
    std::string _alt;
};

inline const Entry& VariantAdaptor::entry() const {
    return _entry;
}

inline uint32_t VariantAdaptor::index() const {
    return _idx;
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

inline const std::string& VariantAdaptor::reference() const {
    return _ref;
}

inline const std::string& VariantAdaptor::variant() const {
    return _alt;
}

VCF_NAMESPACE_END
