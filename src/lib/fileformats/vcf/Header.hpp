#pragma once

#include "CustomType.hpp"
#include "SampleTag.hpp"
#include "common/namespaces.hpp"

#include <boost/unordered_map.hpp>

#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

BEGIN_NAMESPACE(Vcf)

// FIXME: put in a separate file
class SampleNotFoundError : public std::runtime_error {
public:
    SampleNotFoundError(const std::string& what)
        : std::runtime_error(what)
    {}
};

template<typename K, typename V>
struct HeaderMap {
    typedef boost::unordered_map<K, V> type;
};

class Header {
public:
    typedef std::pair<std::string, std::string> RawLine;

    template<typename T>
    static Header fromStream(T& stream) {
        Header rv;
        std::string line;
        while (stream.peek() == '#' && getline(stream, line)) {
            rv.add(line);
        }
        return rv;
    }

    static Header fromString(std::string const& s) {
        std::stringstream ss(s);
        return fromStream(ss);
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
    HeaderMap<std::string, CustomType>::type const& infoTypes() const;
    HeaderMap<std::string, CustomType>::type const& formatTypes() const;
    HeaderMap<std::string, std::string>::type const& filters() const;
    HeaderMap<std::string, SampleTag>::type const& sampleTags() const;
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
    HeaderMap<size_t, size_t>::type const& mirroredSamples() const;

    bool hasDuplicateSamples() const {
        return _hasDuplicateSamples;
    }

    bool isReflected(size_t sampleIdx) const;
    bool isReflection(size_t sampleIdx) const;

protected:
    void parseHeaderLine(std::string const& line);
    size_t addSample(std::string const& name);

protected:
    HeaderMap<std::string, CustomType>::type _infoTypes;
    HeaderMap<std::string, CustomType>::type _formatTypes;
    // filters = name -> description
    HeaderMap<std::string, std::string>::type _filters;
    std::vector<RawLine> _metaInfoLines;
    std::vector<std::string> _sampleNames;
    HeaderMap<std::string, SampleTag>::type _sampleTags;
    bool _headerSeen;
    uint32_t _sourceIndex;

    std::vector<size_t> _sampleSourceCounts;

    // maps sample idx -> the source it is reflecting
    HeaderMap<size_t, size_t>::type _mirroredSamples;

    // maps sample idx -> # of times it is mirrored
    HeaderMap<size_t, bool>::type _hasReflections;

    HeaderMap<std::string, size_t>::type _sampleIndices;
    bool _hasDuplicateSamples;
};

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, Vcf::Header const& h);
