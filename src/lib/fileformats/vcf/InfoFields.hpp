#pragma once

#include "CustomValue.hpp"
#include "common/namespaces.hpp"

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

BEGIN_NAMESPACE(Vcf)

class Header;

class InfoFields {
public:
    typedef std::map<std::string, CustomValue> CustomValueMap;

    // gcc4.4 is awful
    InfoFields();
    InfoFields(InfoFields const& rhs);
    InfoFields(InfoFields&& rhs);
    InfoFields& operator=(InfoFields const& rhs);

    void fromString(std::string text);
    void fromString(char const* first, char const* last) {
        fromString(std::string(first, last));
    }

    CustomValueMap const& get(Header const& header) const;
    CustomValueMap& get(Header const& header);

    void clear() {
        text_.clear();
        data_.reset();
    }

    template<typename OS>
    void toStream(OS& os) const {
        if (data_)
            parsedToStream(os);
        else if (text_.empty())
            os << ".";
        else
            os << text_;
    }

    template<typename OS>
    friend OS& operator<<(OS& os, InfoFields const& info) {
        info.toStream(os);
        return os;
    }

    void swap(InfoFields& rhs) {
        text_.swap(rhs.text_);
        data_.swap(rhs.data_);
    }

private:
    void parse(Header const& header) const;

    template<typename OS>
    void printOne(OS& os, CustomValueMap::const_iterator iter) const {
        os << iter->second.type().id();
        if (!iter->second.empty()) {
            os << "=";
            iter->second.toStream(os);
        }
    }

    template<typename OS>
    void parsedToStream(OS& os) const {
        assert(data_);
        if (data_->empty()) {
            os << ".";
            return;
        }

        auto const& data = *data_;
        auto first = data.begin();
        auto last = data.end();

        if (first != last) {
            printOne(os, first++);
        }

        for (; first != last; ++first) {
            os << ';';
            printOne(os, first);
        }
    }

private:
    std::string text_;
    mutable std::unique_ptr<CustomValueMap> data_;
};

END_NAMESPACE(Vcf)
