#pragma once

#include "common/VariantType.hpp"
#include "common/namespaces.hpp"

#include <cstdint>
#include <string>
#include <vector>

class Fasta;

BEGIN_NAMESPACE(Vcf)
class Entry;

class AltNormalizer {
public:
    typedef Fasta RefSeq;
    struct RefEdit {
        RefEdit() : pos(0), len(0) {}

        RefEdit(uint64_t pos, uint64_t len, uint64_t lastRef, std::string bases, VariantType type)
            : pos(pos), len(len), lastRef(lastRef), bases(bases), type(type)
        {}

        uint64_t pos;
        uint64_t len;
        uint64_t lastRef;
        std::string bases;
        VariantType type;
    };


    AltNormalizer(RefSeq const& ref);

    void normalize(Entry& e);
    RefEdit moveInsertion(Entry& e, size_t idx) const;
    RefEdit moveDeletion(Entry& e, size_t idx) const;

protected:
    RefSeq const& _ref;
};

END_NAMESPACE(Vcf)
