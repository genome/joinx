#pragma once

#include "TypedStream.hpp"
#include "vcf/Entry.hpp"
#include "vcf/Header.hpp"

typedef TypedStream<DefaultParser<Vcf::Entry>> VcfReader;

struct ReheaderingVcfParser {
    typedef Vcf::Entry ValueType;

    Vcf::Header const* newHeader;

    ReheaderingVcfParser(Vcf::Header const* newHeader);
    void operator()(Vcf::Header const* h, std::string& line, Vcf::Entry& entry);
};

VcfReader::ptr openVcf(InputStream& in);

template<typename InputStreamPtrs>
std::vector<VcfReader::ptr> openVcfs(InputStreamPtrs& streams) {
    std::vector<VcfReader::ptr> rv;
    size_t idx = 0;
    for (auto iter = streams.begin(); iter != streams.end(); ++iter) {
        rv.push_back(openVcf(**iter));
        rv.back()->header().sourceIndex(idx++);
    }
    return rv;
}
