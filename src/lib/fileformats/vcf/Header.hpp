#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "CustomType.hpp"
#include "SampleTag.hpp"
#include "common/namespaces.hpp"

BEGIN_NAMESPACE(Vcf)

// FIXME: put in a separate file
class SampleNotFoundError : public std::runtime_error {
public:
    SampleNotFoundError(const std::string& what)
        : std::runtime_error(what)
    {}
};

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

    void add(std::string const& line);
    void addFilter(std::string const& name, std::string const& desc);
    void addInfoType(CustomType const& type);
    void addFormatType(CustomType const& type);
    void addSampleTag(SampleTag const& tag);
    void merge(const Header& other, bool allowDuplicateSamples = false);
    bool empty() const;

    const std::vector<RawLine>& metaInfoLines() const;
    std::string headerLine() const;
    // infoType/formatType return NULL for non-existing ids
    CustomType const* infoType(std::string const& id) const;
    CustomType const* formatType(std::string const& id) const;
    SampleTag const* sampleTag(std::string const& id) const;
    std::map<std::string, CustomType> const& infoTypes() const;
    std::map<std::string, CustomType> const& formatTypes() const;
    std::map<std::string, std::string> const& filters() const;
    std::map<std::string, SampleTag> const& sampleTags() const;
    std::vector<std::string> const& sampleNames() const;

    uint32_t sampleCount() const { return _sampleNames.size(); }

    // throws when sampleName is not found
    uint32_t sampleIndex(std::string const& sampleName) const;

    void sourceIndex(uint32_t value) { _sourceIndex = value; }
    uint32_t sourceIndex() const { return _sourceIndex; }

    // will throw if the header is invalid
    void assertValid() const;

    std::vector<size_t> const& sampleSourceCounts() const;

    // Sample mirroring effectively duplicates a given sample column giving it a
    // new name. This is useful for joinx vcf-merge, which has an option to
    // preserve the sample input columns in the output as well as producing the
    // merged column.
    void mirrorSample(std::string const& sampleName, std::string const& newName);
    std::map<size_t, size_t> const& mirroredSamples() const;

    bool hasDuplicateSamples() const {
        return _hasDuplicateSamples;
    }

protected:
    void parseHeaderLine(std::string const& line);
    size_t addSample(std::string const& name);

protected:
    std::map<std::string, CustomType> _infoTypes;
    std::map<std::string, CustomType> _formatTypes;
    // filters = name -> description
    std::map<std::string, std::string> _filters;
    std::vector<RawLine> _metaInfoLines;
    std::vector<std::string> _sampleNames;
    std::map<std::string, SampleTag> _sampleTags;
    bool _headerSeen;
    uint32_t _sourceIndex;

    std::vector<size_t> _sampleSourceCounts;

    std::map<size_t, size_t> _mirroredSamples;
    std::unordered_map<std::string, size_t> _sampleIndices;
    bool _hasDuplicateSamples;
};

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, Vcf::Header const& h);
