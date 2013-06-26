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
        //auto const& sampleNames = _reader.header().sampleNames();
        while (_reader.next(entry)) {
            std::vector<Vcf::RawVariant> rawVars = Vcf::RawVariant::processEntry(entry);
            for (auto vi = rawVars.begin(); vi != rawVars.end(); ++vi) {
                _callback(entry.chrom(), *vi);
            }
/*
            auto const sampleData = entry.sampleData();
            for (size_t i = 0; i < sampleNames.size(); ++i) {
                auto const& sampleName = sampleNames[i];
                GenotypeCall const& gt = sampleData.genotype(i);
                if (gt == GenotypeCall::Null)
                    continue;

                for (auto i = gt.begin(); i != gt.end(); ++i) {
                callback(sampleName, 
            }
*/
        }
    }

private:
    VcfReader& _reader;
    RawCallback _callback;
};
