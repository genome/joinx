#pragma once

#include "TypedStream.hpp"
#include "vcf/Entry.hpp"
#include "vcf/Header.hpp"

#include <functional>
#include <string>

namespace {
typedef std::function<void(const Vcf::Header*, std::string&, Vcf::Entry&)> VcfExtractor;
typedef TypedStream<Vcf::Entry, VcfExtractor> VcfReader;
typedef std::function<VcfReader::ptr(InputStream&)> VcfOpenerType;
}

VcfReader::ptr openVcf(InputStream& in);
