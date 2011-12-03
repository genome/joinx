#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "CustomType.hpp"
#include "common/namespaces.hpp"

BEGIN_NAMESPACE(Vcf)

class Header {
public:
    typedef std::pair<std::string, std::string> RawLine;

    template<typename T>
    static Header fromStream(T& stream) {
        Header rv;
        std::string line;
        while (stream.peek() == '#' && getline(stream, line))
            rv.add(line);
        return rv;
    }

    Header();
    ~Header();

    void add(const std::string& line);
    void addFilter(const std::string& name, const std::string& desc);
    void merge(const Header& other, bool allowDuplicateSamples = false);
    bool empty() const;

    const std::vector<RawLine>& metaInfoLines() const;
    std::string headerLine() const;
    // infoType/formatType return NULL for non-existing ids
    const CustomType* infoType(const std::string& id) const;
    const CustomType* formatType(const std::string& id) const;
    const std::map<std::string, CustomType>& infoTypes() const;
    const std::map<std::string, CustomType>& formatTypes() const;
    const std::map<std::string, std::string>& filters() const;
    const std::vector<std::string>& sampleNames() const;

    uint32_t sampleCount() const { return _sampleNames.size(); }

    // throws when sampleName is not found
    uint32_t sampleIndex(const std::string& sampleName) const;

    void sourceIndex(uint32_t value) { _sourceIndex = value; }
    uint32_t sourceIndex() const { return _sourceIndex; }

    // will throw if the header is invalid
    void assertValid() const;

protected:
    void parseHeaderLine(const std::string& line);

protected:
    std::map<std::string, CustomType> _infoTypes;
    std::map<std::string, CustomType> _formatTypes;
    // filters = name -> description
    std::map<std::string, std::string> _filters;
    std::vector<RawLine> _metaInfoLines;
    std::vector<std::string> _sampleNames;
    bool _headerSeen;
    uint32_t _sourceIndex;
};

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h);
