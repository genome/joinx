#include "FastaIndex.hpp"

#include "common/Tokenizer.hpp"

#include <boost/format.hpp>

#include <stdexcept>
#include <cctype>

using boost::format;

FastaIndex::Entry::Entry()
    : len(0)
    , offset(0)
    , lineBasesLength(0)
    , lineLength(0)
{
}

FastaIndex::Entry::Entry(std::string const& data) {
    Tokenizer<char> tok(data, '\t');
    if (!tok.extract(name)
        || !tok.extract(len)
        || !tok.extract(offset)
        || !tok.extract(lineBasesLength)
        || !tok.extract(lineLength))
    {
        throw std::runtime_error(str(format(
            "Failed to parse fasta index line '%1%'") %data));
    }
}

FastaIndex::FastaIndex() {
}

FastaIndex::FastaIndex(std::istream& in) {
    std::string line;
    while (getline(in, line)) {
        Entry e(line);
        _sequenceOrder.push_back(e.name);
        _entries[e.name] = e;
    }
}

std::vector<std::string> const& FastaIndex::sequenceOrder() const {
    return _sequenceOrder;
}

auto FastaIndex::entry(std::string const& name) const -> Entry const* {
    auto iter = _entries.find(name);
    if (iter != _entries.end())
        return &iter->second;
    return 0;
}

void FastaIndex::save(std::ostream& out) {
    for (auto i = _entries.begin(); i != _entries.end(); ++i) {
        out << i->second.name << "\t"
            << i->second.len << "\t"
            << i->second.offset << "\t"
            << i->second.lineBasesLength << "\t"
            << i->second.lineLength << "\n";
    }
}
