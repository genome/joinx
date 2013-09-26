#pragma once

#include "TypedStream.hpp"
#include "vcf/Entry.hpp"
#include "vcf/Header.hpp"

#include <boost/function.hpp>

#include <functional>
#include <string>
#include <vector>

namespace {
typedef boost::function<void(const Vcf::Header*, std::string&, Vcf::Entry&)> VcfExtractor;
typedef TypedStream<Vcf::Entry, VcfExtractor> VcfReader;
typedef boost::function<VcfReader::ptr(InputStream&)> VcfOpenerType;
}

VcfReader::ptr openVcf(InputStream& in);

template<typename InputStreamPtrs>
std::vector<VcfReader::ptr> openVcfs(InputStreamPtrs& streams) {
    std::vector<VcfReader::ptr> rv;
    for (auto iter = streams.begin(); iter != streams.end(); ++iter) {
        rv.push_back(openVcf(**iter));
    }
    return rv;
}
