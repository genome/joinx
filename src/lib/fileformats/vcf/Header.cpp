#include "Header.hpp"
#include "Map.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <utility>

using boost::format;
using namespace std;
using namespace std::placeholders;

VCF_NAMESPACE_BEGIN

namespace {
    const char *expectedHeaderFields[] = {
        "CHROM", "POS", "ID", "REF", "ALT",
        "QUAL", "FILTER", "INFO", "FORMAT"
    };

    unsigned nExpectedHeaderFields = sizeof(expectedHeaderFields)/sizeof(expectedHeaderFields[0]);
}

Header::Header() : _headerSeen(false) {}

void Header::add(const string& line) {
    // determine if this is a meta-information line (##) or header line (#)
    if (line[1] == '#') {
        // meta-info
        Tokenizer<char> t(line, '=');
        pair<string, string> p;
        if (!t.extract(p.first))
            throw runtime_error(str(format("Failed to parse VCF header line: %1%") %line));
        p.first = p.first.substr(2); // strip leading ##
        t.remaining(p.second);

        if (p.first == "INFO") {
            CustomType t(p.second.substr(1, p.second.size()-2));
            auto inserted = _infoTypes.insert(make_pair(t.id(), t));
            if (!inserted.second)
                throw runtime_error(str(format("Duplicate value for INFO:%1%") %t.id()));
        } else if (p.first == "FORMAT") {
            CustomType t(p.second.substr(1, p.second.size()-2));
            auto inserted = _formatTypes.insert(make_pair(t.id(), t));
            if (!inserted.second)
                throw runtime_error(str(format("Duplicate value for FORMAT:%1%") %t.id()));
        } else if (p.first == "FILTER") {
            // TODO: care about duplicates?
            Map m(p.second.substr(1, p.second.size()-2));
            _filters.insert(make_pair(m["ID"], m["Description"]));
        }

        _metaInfoLines.push_back(p);
    } else {
        parseHeaderLine(line.substr(1));
    }
}

void Header::parseHeaderLine(const std::string& line) {
    if (_headerSeen)
        throw runtime_error(str(format("Multiple header line detected:\n%1%\nAND\n%2%") %line %line));

    _headerSeen = true;
    Tokenizer<char> t(line, '\t');
    string tok;
    for (unsigned i = 0; i < nExpectedHeaderFields; ++i) {
        const char* expected = expectedHeaderFields[i];
        if (!t.extract(tok) || tok != expected)
            throw runtime_error(str(format("Malformed header line: %1%\nExpected token: %2%") %line %expected));
    }

    while (t.extract(tok))
        _sampleNames.push_back(tok);
}

inline std::string Header::headerLine() const {
    stringstream ss;
    ss << expectedHeaderFields[0];
    for (unsigned i = 1; i < nExpectedHeaderFields; ++i)
        ss << "\t" << expectedHeaderFields[i];
    for (auto iter = _sampleNames.begin(); iter != _sampleNames.end(); ++iter)
        ss << "\t" << *iter;
    return ss.str();
}


void Header::assertValid() const {
    if (_metaInfoLines.empty() || !_headerSeen)
        throw runtime_error("invalid or missing header");
}

void Header::merge(const Header& other) {
    for (auto iter = other._metaInfoLines.begin(); iter != other._metaInfoLines.end(); ++iter) {
        // we already have that exact line
        if (find(_metaInfoLines.begin(), _metaInfoLines.end(), *iter) != _metaInfoLines.end())
            continue;
        add("##"+iter->first+"="+iter->second);
    }

    for (auto iter = other._sampleNames.begin(); iter != other._sampleNames.end(); ++iter) {
        if (find(_sampleNames.begin(), _sampleNames.end(), *iter) != _sampleNames.end())
            throw runtime_error(str(format("Error merging VCF headers, sample name conflict: %1%") %*iter));
        _sampleNames.push_back(*iter);
    }
}

unsigned Header::sampleIndex(const std::string& sampleName) {
    auto iter = find(_sampleNames.begin(), _sampleNames.end(), sampleName);
    if (iter == _sampleNames.end())
        throw runtime_error(str(format("Request for sample name '%1%' which does not exist in this VCF header") %sampleName));
    return distance(_sampleNames.begin(), iter);
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h) {
    const vector<Vcf::Header::RawLine>& mlines = h.metaInfoLines();
    for (auto i = mlines.begin(); i != mlines.end(); ++i)
        s << "##" << i->first << "=" << i->second << "\n";

    if (!h.headerLine().empty())
        s << '#' << h.headerLine() << "\n";

    return s;
}
