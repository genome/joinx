#pragma once

#include "fileformats/Bed.hpp"
#include "fileformats/Variant.hpp"

#include <functional>
#include <set>
#include <vector>

template<typename OutputType>
class BedDeduplicator {
public:
    typedef Bed ValueType;

    BedDeduplicator(OutputType& out)
        : _out(out)
    {
    }

    ~BedDeduplicator() {
        flush();
    }

    bool posCmp(const ValueType& a, const ValueType& b) const {
        return a.chrom() == b.chrom()
            && a.start() == b.start()
            && a.stop() == b.stop();
    }

    bool haveAlleles(const ValueType& value) const {
        if (value.extraFields().empty())
            return true;

        for (auto i = _cache.begin(); i != _cache.end(); ++i) {
            if (!i->extraFields().empty()
                && value.extraFields()[0] == i->extraFields()[0])
            {
                return true;
            }
        }

        return false;
    }

    bool cache(const ValueType& value) {
        if (_cache.empty() || posCmp(_cache.front(), value)) {
            _cache.push_back(value);
            return true;
        }
        return false;
    }

    void flush() {
        std::set<std::string> seen;
        for (auto i = _cache.begin(); i != _cache.end(); ++i) {
            if (i->extraFields().empty()) {
                auto inserted = seen.insert("");
                if (inserted.second)
                    _out(*i);
            } else {
                auto inserted = seen.insert(i->extraFields()[0]);
                if (inserted.second)
                    _out(*i);
            }
        }
        _cache.clear();
    }

    void operator()(const ValueType& value) {
        if (cache(value)) {
            return;
        }
        flush();
        cache(value);
    }

protected:
    OutputType& _out;
    std::vector<ValueType> _cache;
};
