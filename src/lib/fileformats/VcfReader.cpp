#include "VcfReader.hpp"

#include "common/compat.hpp"

ReheaderingVcfParser::ReheaderingVcfParser(Vcf::Header const* newHeader)
    : newHeader(newHeader)
{}

void ReheaderingVcfParser::operator()(Vcf::Header const* h, std::string& line, Vcf::Entry& entry) {
    return Vcf::Entry::parseLineAndReheader(h, newHeader, line, entry);
}

VcfReader::ptr openVcf(InputStream& in) {
    return TypedStreamFactory<DefaultParser<Vcf::Entry>>{}(in);
}
