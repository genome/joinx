#pragma once

#include "fileformats/vcf/Header.hpp"
#include "fileformats/vcf/Entry.hpp"
#include "fileformats/vcf/GenotypeCall.hpp"
#include "fileformats/vcf/RawVariant.hpp"

template<typename VcfReader, typename RawCallback>
class VcfToRaw {
public:
    typedef void(*CallbackType)(std::string const&, Vcf::RawVariant const&);

    VcfToRaw(VcfReader& reader, RawCallback callback)
        : _reader(reader)
        , _callback(callback)
    {
    }

    void convert() {
        typename VcfReader::ValueType entry;
        while (_reader.next(entry)) {
            auto rawVars = Vcf::RawVariant::processEntry(entry);
            for (auto vi = rawVars.begin(); vi != rawVars.end(); ++vi) {
                _callback(entry.chrom(), *vi);
            }
        }
    }

private:
    VcfReader& _reader;
    RawCallback _callback;
};
