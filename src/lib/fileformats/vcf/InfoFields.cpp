#include "InfoFields.hpp"

#include "Header.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>

#include <iterator>
#include <vector>

BEGIN_NAMESPACE(Vcf)


InfoFields::InfoFields(Header const& h, std::string const& s, std::size_t numAlts) {
    using boost::format;

    std::vector<std::string> infoStrings;
    auto beg = s.c_str();
    auto end = beg + s.size();

    if (beg != end && s != ".")
        Tokenizer<char>::split(beg, end, ';', std::back_inserter(infoStrings));

    for (auto i = infoStrings.begin(); i != infoStrings.end(); ++i) {
        if (i->empty())
            continue;

        Tokenizer<char> kv(*i, '=');
        std::string key;
        std::string value;
        kv.extract(key);
        kv.remaining(value);
        CustomType const* type = h.infoType(key);
        if (type == NULL) {
            throw std::runtime_error(str(format(
                "Failed to lookup type for info field '%1%'"
                ) % key));
        }

        CustomValue cv(type, value);
        cv.setNumAlts(numAlts);
        auto inserted = data_.insert(make_pair(key, cv));
        if (!inserted.second)
            throw std::runtime_error(str(format(
                "Duplicate value for info field '%1%'"
                ) % key));
    }
}

auto InfoFields::operator*() const -> MapType const& {
    return data_;
}

auto InfoFields::operator*() -> MapType& {
    return data_;
}

END_NAMESPACE(Vcf)
