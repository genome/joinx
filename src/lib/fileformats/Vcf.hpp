#pragma once

#include <common/Tokenizer.hpp>
#include <fileformats/Variant.hpp>

#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <utility>

#define VCF_NAMESPACE_BEGIN namespace Vcf {
#define VCF_NAMESPACE_END   }

VCF_NAMESPACE_BEGIN

enum DataType {
    INT,
    FLOAT,
    CHAR,
    STRING,
    FLAG
};

DataType strToType(const std::string& s);
std::string parseString(const std::string& s);

class Header {
public:
// Nested types
    typedef std::pair<std::string, std::string> RawLine;

    class PropLine {
    public:
        PropLine();
        PropLine(const std::string& s, const std::vector<std::string> validProps);
        const std::vector<std::string>& keys() const;
        const std::string& operator[](const std::string& key);

    protected:
        std::vector<std::string> _keyOrder;
        std::map<std::string, std::string> _map;
    };

// Functions
    Header();

    void add(const std::string& line);

    const std::vector<RawLine>& lines() const;

protected:
    std::vector<RawLine> _lines;
    std::vector<PropLine> _info;
    std::vector<PropLine> _filters;
    std::vector<PropLine> _format;
};

class InfoField {
public:
    InfoField();
    InfoField(const std::string& raw);
    InfoField(const std::string& id, const std::string& val);

    const std::string& id() const;
    const std::string& value() const;

    template<typename T>
    T as() const { return boost::lexical_cast<T>(_value); }

protected:
    std::string _id;
    std::string _value;
};


class Entry {
public:
    Entry();
    explicit Entry(const std::string& s);

    void parse(const std::string& s);

    void setIdentifiers(const std::string& s);

    const std::string& line() const { return _line; }
    const std::string& chrom() const { return _chrom; }
    uint64_t pos() const { return _pos; }
    const std::vector<std::string>& identifiers() const { return _identifiers; }
    const std::string& ref() const { return _ref; }
    const std::vector<std::string>& alt() const { return _alt; }
    float qual() const { return _qual; }
    const std::vector<std::string>& failedFilters() const { return _failedFilters; }
    const std::vector<InfoField>& info() const { return _info; }
    const std::vector<std::string>& formatDescription() const { return _formatDescription; }
    const std::vector< std::vector<std::string> >& perSampleData() const { return _perSampleData; }

    int64_t start() const;
    int64_t stop() const;
    int64_t length() const { return stop() - start(); }
    std::string toString() const;

    std::vector<Variant> variants() const;

    template<typename T>
    void extractList(T& v, const std::string& s) {
        if (s == ".")
            return;

        Tokenizer<char> t(s, ';');
        typename T::value_type tmp;
        while (t.extract(tmp)) {
            v.push_back(tmp);
        }
    }

    template<typename T>
    void printList(std::ostream& s, const T& v) const {
        if (!v.empty()) {
            for (auto i = v.begin(); i != v.end(); ++i) {
                if (i != v.begin())
                    s << ';';
                s << *i;
            }
        } else {
            s << '.';
        }
    }

protected:
    std::string _line;
    std::string _chrom;
    uint64_t _pos;
    std::vector<std::string> _identifiers;
    std::string _ref;
    std::vector<std::string> _alt;
    float _qual;
    std::vector<std::string> _failedFilters;
    std::vector<InfoField> _info;
    std::vector<std::string> _formatDescription;
    std::vector< std::vector<std::string> > _perSampleData;
};

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h);
std::ostream& operator<<(std::ostream& s, const Vcf::InfoField& i);
std::ostream& operator<<(std::ostream& s, const Vcf::Entry& e);
