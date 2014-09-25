#pragma once

#include "CustomValue.hpp"
#include "common/namespaces.hpp"

#include <boost/container/flat_map.hpp>

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

BEGIN_NAMESPACE(Vcf)

class Header;

class InfoFields {
public:
    typedef std::map<std::string, CustomValue> MapType;

    InfoFields(Header const& h, std::string const& s, std::size_t numAlts);
    MapType const& operator*() const;
    MapType& operator*();

    void clear() {
        data_.clear();
    }

    template<typename OS>
    friend OS& operator<<(OS& os, InfoFields const& info) {
        info.toStream(os);
        return os;
    }

private:
    void parse(Header const& header) const;

    template<typename OS>
    void printOne(OS& os, MapType::const_iterator iter) const {
        os << iter->second.type().id();
        if (!iter->second.empty()) {
            os << "=";
            iter->second.toStream(os);
        }
    }

    template<typename OS>
    void toStream(OS& os) const {
        if (data_.empty()) {
            os << ".";
            return;
        }

        auto first = data_.begin();
        auto last = data_.end();

        if (first != last) {
            printOne(os, first++);
        }

        for (; first != last; ++first) {
            os << ';';
            printOne(os, first);
        }
    }
private:
    MapType data_;
};

END_NAMESPACE(Vcf)
