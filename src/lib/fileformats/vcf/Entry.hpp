#pragma once

#include "CustomValue.hpp"
#include "GenotypeCall.hpp"
#include "common/Tokenizer.hpp"
#include "common/namespaces.hpp"

#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include <set>

//TODO needs addFilter function. Make sure to check if filter is available in header
BEGIN_NAMESPACE(Vcf)

class EntryMerger;
class Header;

class Entry {
public:
    enum FieldName {
        CHROM,
        POS,
        ID,
        REF,
        ALT,
        QUAL,
        FILTER,
        INFO,
        FORMAT,
        SAMPLE_DATA,
        UNDEFINED
    };
    typedef Header HeaderType;
    typedef std::map<std::string, CustomValue> CustomValueMap;
    typedef std::map<uint32_t, std::vector<CustomValue> > SampleData;

    // static data
    const static double MISSING_QUALITY;

    // static functions
    static const char* fieldToString(FieldName field);
    static FieldName fieldFromString(const char* name);
    static void parseLine(const Header* hdr, std::string& s, Entry& e);
    static void parseLineAndReheader(const Header* hdr, const Header* newH, std::string& s, Entry& e);

    // member functions
    Entry();
    // for move semantics
    Entry(Entry&& e) throw();
    explicit Entry(const Header* h);
    Entry(EntryMerger&& merger);
    Entry(const Header* h, const std::string& s);

    // This updates the entry to use the new header instead. This can
    // move the sample data around, but does not currently do any more
    // validation.
    void reheader(const Header* newHeader);

    const Header& header() const;
    void parse(const Header* h, const std::string& s);
    void parseAndReheader(const Header* h, const Header* newH, const std::string& s);

    void addIdentifier(const std::string& id);
    static bool isInvalidFilterId (char c) {
        return (c == ';' || isspace(c));
    }

    void addFilter(const std::string& filterName);

    const std::string& chrom() const { return _chrom; }
    const uint64_t& pos() const { return _pos; }
    const std::set<std::string>& identifiers() const { return _identifiers; }
    const std::string& ref() const { return _ref; }
    const std::vector<std::string>& alt() const { return _alt; }
    double qual() const { return _qual; }
    const std::set<std::string>& failedFilters() const { return _failedFilters; }
    const CustomValueMap& info() const { return _info; }
    const std::vector<CustomType const*>& formatDescription() const { return _formatDescription; }
    const std::vector<CustomValue>* sampleData(uint32_t idx) const;
    const CustomValue* info(const std::string& key) const;
    const SampleData& sampleData() const;
    const CustomValue* sampleData(uint32_t sampleIdx, const std::string& key) const;

    // returns true if GT is the first FORMAT entry
    bool hasGenotypeData() const;
    GenotypeCall genotypeForSample(uint32_t sampleIdx) const;

    uint32_t samplesWithData() const;
    int32_t samplesFailedFilter() const;
    int32_t samplesEvaluatedByFilter() const;
    void removeLowDepthGenotypes(uint32_t lowDepth);

    void setPositions();
    int64_t start() const;
    int64_t stop() const;
    int64_t length() const { return stop() - start(); }

    // -1 if not found
    int32_t altIdx(const std::string& alt) const;

    std::string toString() const;

    template<typename T>
    void printList(std::ostream& s, const T& v, char delim = ';') const {
        if (!v.empty()) {
            for (auto i = v.begin(); i != v.end(); ++i) {
                if (i != v.begin())
                    s << delim;
                s << *i;
            }
        } else {
            s << '.';
        }
    }

    int cmp(const Entry& rhs) const;
    bool operator<(const Entry& rhs) const;

    void swap(Entry& other);

protected:
    SampleData& sampleData();
    // not really const, but we want to call from const functions
    // so we make sampleData mutable
    void parseSamples() const;

protected:
    const Header* _header;
    std::string _chrom;
    uint64_t _pos;
    std::set<std::string> _identifiers;
    std::string _ref;
    std::vector<std::string> _alt;
    double _qual;
    std::set<std::string> _failedFilters;
    CustomValueMap _info;
    std::vector<CustomType const*> _formatDescription;
    std::string _sampleString;
    mutable bool _parsedSamples;
    mutable SampleData _sampleData;

    int64_t _start;
    int64_t _stop;
};

inline bool Entry::operator<(const Entry& rhs) const {
    return cmp(rhs) < 0;
}

END_NAMESPACE(Vcf)

std::ostream& operator<<(std::ostream& s, const Vcf::Entry& e);
