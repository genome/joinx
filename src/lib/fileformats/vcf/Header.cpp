#include "Header.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

Header::Header() {}

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
        _metaInfoLines.push_back(p);

        if (p.second[0] == '<' && p.second[p.second.size()-1] == '>') {
            string value = p.second.substr(1, p.second.size()-2); // strip wrapping <>
            _categoryNames.insert(p.first);
            _categories[p.first].push_back(Map(value));
        }
    } else {
        _headerLines.push_back(line.substr(1));
    }
}

void Header::assertValid() const {
    if (_metaInfoLines.empty())
        throw runtime_error("invalid or missing header");
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h) {
    const vector<Vcf::Header::RawLine>& mlines = h.metaInfoLines();
    for (auto i = mlines.begin(); i != mlines.end(); ++i)
        s << "##" << i->first << "=" << i->second << "\n";

    const vector<string>& hlines = h.headerLines();
    for (auto i = hlines.begin(); i != hlines.end(); ++i)
        s << "#" << *i << "\n";

    return s;
}
