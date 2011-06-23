#pragma once

#include "common/Variant.hpp"

#include <iostream>
#include <string>
#include <boost/format.hpp>

template<typename RefSeq, typename VariantStream>
class VariantsToContigs {
public:
    VariantsToContigs(
            RefSeq& ref,
            VariantStream& var,
            std::ostream& out,
            unsigned flankSize,
            int minQuality = 0)
        : _ref(ref)
        , _var(var)
        , _out(out)
        , _flankSize(flankSize)
        , _minQuality(minQuality)
    {
    }

    void execute() {
        typename VariantStream::ValueType raw;
        while (_var.next(raw)) {
            Variant v(raw);
            if (v.quality() >= _minQuality)
                processVariant(v);
        }
    }

    void processVariant(const Variant& v) {
        using boost::format;

        unsigned start = 0;
        if (v.start() > _flankSize)
            start = v.start() - _flankSize;

        unsigned stop = v.stop() + _flankSize;
        std::string seq;
        unsigned len = stop-start;
        seq.reserve(len);

        for (unsigned i = start; i < v.start(); ++i)
            seq.append(1, _ref.sequence(v.chrom(), i));

        if (v.type() != Variant::DEL)
            seq.append(v.variant().data());

        for (unsigned i = v.stop(); i < stop; ++i)
            seq.append(1, _ref.sequence(v.chrom(), i));

        std::string cigar;
        switch (v.type()) {
        case Variant::SNP:
            cigar = str(format("%1%M") %len);
            break;

        case Variant::DNP:
        case Variant::INS:
            cigar = str(format("%1%M%2%I%3%M") 
                %(v.start()-start)
                %v.variant().data().size()
                %(stop-v.stop())
            );
            break;

        case Variant::DEL:
            cigar = str(format("%1%M%2%D%3%M")
                %(v.start()-start)
                %v.reference().data().size()
                %(stop-v.stop()));
            break;

        default:
            break;
        }

        std::string name(str(format("REMAP-%1%,%2%,%3% %4%") %v.chrom() %start %stop %cigar));
        _out << "@ " << name << "\n" << seq << "\n";
    }

protected:
    RefSeq& _ref;
    VariantStream& _var;
    std::ostream& _out;
    unsigned _flankSize;
    int _minQuality;
};
