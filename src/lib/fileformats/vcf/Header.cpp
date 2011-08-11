#include "Header.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>
#include <stdexcept>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

Header::Header() {}

void Header::add(const string& line) {
    Tokenizer<char> t(line, '=');
    pair<string, string> p;
    if (!t.extract(p.first))
        throw runtime_error(str(format("Failed to parse VCF header line: %1%") %line));
    p.first = p.first.substr(2); // strip leading ##
    t.remaining(p.second);
    _lines.push_back(p);

    if (p.second[0] == '<' && p.second[p.second.size()-1] == '>') {
        string value = p.second.substr(1, p.second.size()-2); // strip wrapping <>
        _categoryNames.insert(p.first);
        _categories[p.first].push_back(Map(value));
    }
}

VCF_NAMESPACE_END

std::ostream& operator<<(std::ostream& s, const Vcf::Header& h) {
    const vector<Vcf::Header::RawLine>& lines = h.lines();
    for (auto i = lines.begin(); i != lines.end(); ++i)
        s << "##" << i->first << "=" << i->second << "\n";
    return s;
}
