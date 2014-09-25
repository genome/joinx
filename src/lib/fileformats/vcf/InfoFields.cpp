#include "InfoFields.hpp"

#include "Header.hpp"
#include "common/Tokenizer.hpp"

#include <boost/format.hpp>

#include <iterator>
#include <vector>

BEGIN_NAMESPACE(Vcf)

InfoFields::InfoFields() {}
InfoFields::InfoFields(InfoFields const& rhs)
    : text_(rhs.text_)
{
    if (rhs.data_)
        data_.reset(new CustomValueMap(*rhs.data_));
}

InfoFields& InfoFields::operator=(InfoFields const& rhs) {
    text_ = rhs.text_;

    if (rhs.data_)
        data_.reset(new CustomValueMap(*rhs.data_));

    return *this;
}

InfoFields::InfoFields(InfoFields&& rhs)
    : text_(std::move(rhs.text_))
    , data_(std::move(rhs.data_))
{}



void InfoFields::fromString(std::string text) {
    text_ = std::move(text);
    data_.reset();
}

auto InfoFields::get(Header const& header) const -> CustomValueMap const& {
    parse(header);
    return *data_;
}

auto InfoFields::get(Header const& header) -> CustomValueMap& {
    parse(header);
    return *data_;
}

void InfoFields::parse(Header const& header) const {
    using boost::format;

    if (data_)
        return;

    data_.reset(new CustomValueMap());

    std::vector<std::string> infoStrings;
    auto beg = text_.c_str();
    auto end = beg + text_.size();

    if (beg != end && text_ != ".")
        Tokenizer<char>::split(beg, end, ';', std::back_inserter(infoStrings));

    for (auto i = infoStrings.begin(); i != infoStrings.end(); ++i) {
        if (i->empty())
            continue;

        Tokenizer<char> kv(*i, '=');
        std::string key;
        std::string value;
        kv.extract(key);
        kv.remaining(value);
        CustomType const* type = header.infoType(key);
        if (type == NULL) {
            throw std::runtime_error(str(format(
                "Failed to lookup type for info field '%1%'"
                ) % key));
        }

        CustomValue cv(type, value);
        //cv.setNumAlts(_alt.size());
        auto inserted = data_->insert(make_pair(key, cv));
        if (!inserted.second)
            throw std::runtime_error(str(format(
                "Duplicate value for info field '%1%'"
                ) % key));
    }
}

END_NAMESPACE(Vcf)
