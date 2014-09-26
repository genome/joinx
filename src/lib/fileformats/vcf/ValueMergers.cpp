#include "ValueMergers.hpp"

#include "CustomType.hpp"
#include "CustomValue.hpp"
#include "Entry.hpp"
#include "common/Tokenizer.hpp"
#include "common/compat.hpp"
#include "io/StreamJoin.hpp"

#include <boost/unordered_set.hpp>
#include <boost/format.hpp>

#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)

namespace {
    boost::unordered_set<std::string> valueToHash(CustomValue const& v) {
        boost::unordered_set<std::string> values;
        for (size_t i = 0; i < v.size(); ++i) {
            values.insert(v.getString(i));
        }
        return values;
    }
}

namespace ValueMergers {

std::unique_ptr<Registry> Registry::_instance;

Registry::Registry() {
    registerMerger(std::make_unique<UseFirst const>());
    registerMerger(std::make_unique<UseEarliest const>());
    registerMerger(std::make_unique<EnforceEquality const>());
    registerMerger(std::make_unique<EnforceEqualityUnordered const>());
    registerMerger(std::make_unique<Ignore const>());
    registerMerger(std::make_unique<Sum const>());
    registerMerger(std::make_unique<UniqueConcat const>());
    registerMerger(std::make_unique<PerAltDelimitedList const>());
}

void Registry::registerMerger(Base::const_ptr merger) {
    auto inserted = _mergers.insert(make_pair(merger->name(), std::move(merger)));
    if (!inserted.second)
        throw runtime_error(str(format("Attempted to register duplicate value merger '%1%'") %merger->name()));
}

Base const* Registry::getMerger(const std::string& name) const {
    auto iter = _mergers.find(name);
    if (iter == _mergers.end())
        throw runtime_error(str(format("Unknown value merger '%1%'") %name));
    return iter->second.get();
}

const Registry* Registry::getInstance() {
    if (!_instance)
        _instance = std::make_unique<Registry>();
    return _instance.get();
}

CustomValue UseFirst::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
    const CustomValue* v(fetch(begin));
    if (!v)
        return CustomValue();
    return *v;
}

CustomValue UseEarliest::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
    for (; begin != end; ++begin) {
        const CustomValue* v(fetch(begin));
        if (v) {
            return *v;
        }
    }
    return CustomValue();
}

CustomValue UniqueConcat::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
        CustomValue rv;
        // find first non-null entry
        const CustomValue* v(NULL);
        while (!v && begin != end)
            v = fetch(begin++);
        if (!v)
            return rv;

        rv = *v;
        set<string> seen;
        for (CustomValue::SizeType i = 0; i < v->size(); ++i)
            seen.insert(v->getString(i));
        for (Entry const* e = begin; e != end; ++e) {
            const CustomValue *v = fetch(e);
            if (v) {
                for (CustomValue::SizeType i = 0; i < v->size(); ++i) {
                    string s = v->getString(i);
                    auto inserted = seen.insert(s);
                    if (inserted.second) {
                        rv.append(CustomValue(&v->type(), s));
                    }
                }
            }
        }
        return rv;
}

CustomValue EnforceEquality::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
    CustomValue rv;
    for (Entry const* e = begin; e != end; ++e) {
        const CustomValue* v = fetch(e);
        if (!v)
            continue;
        if (rv.empty())
            rv = *v;
        else if (rv != *v) {
            throw runtime_error(str(format("Equality condition failed for field %1%: %2% vs %3%")
                %type->id() %rv.toString() %v->toString()));
        }
    }
    return rv;
}

CustomValue EnforceEqualityUnordered::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
    CustomValue rv;
    boost::unordered_set<std::string> values;
    for (Entry const* e = begin; e != end; ++e) {
        const CustomValue* v = fetch(e);
        if (!v)
            continue;

        if (rv.empty()) {
            rv = *v;
            values = valueToHash(*v);
        }
        else if (values != valueToHash(*v)) {
            throw runtime_error(str(format("Unordered equality condition failed for field %1%: %2% vs %3%")
                %type->id() %rv.toString() %v->toString()));
        }
    }
    return rv;
}

CustomValue Sum::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
    CustomValue rv(type);
    for (Entry const* e = begin; e != end; ++e) {
        const CustomValue* v = fetch(e);
        if (!v || v->empty())
            continue;
        rv += *v;
    }
    return rv;
}

CustomValue Ignore::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
    return CustomValue();
}

CustomValue PerAltDelimitedList::operator()(
    CustomType const* type,
    FetchFunc fetch,
    Entry const* begin,
    Entry const* end,
    AltIndices const& newAltIndices
    ) const
{
    // FIXME: make into merge strategy param
    // requires ability to specify merge strategy params in the first place
    static const char* delim = "/";

    if (type->numberType() != CustomType::PER_ALLELE) {
        throw runtime_error(str(format(
            "%1% merge strategy used with something other than per-allele type (%2%)"
            ) % name() % CustomType::numberToString(type->numberType(), type->number())));
    }
    CustomValue rv(type);
    boost::unordered_map<size_t, std::set<std::string>> newValues;

    size_t i(0);
    for (Entry const* e = begin; e != end; ++e, ++i) {
        CustomValue const* v = fetch(e);
        if (!v || v->empty())
            continue;

        for (size_t j = 0; j < v->size(); ++j) {
            auto idx = newAltIndices[i][j];
            Tokenizer<std::string>::split(v->getString(j), delim,
                    std::inserter(newValues[idx], newValues[idx].begin()));
        }
    }

    for (auto iter = newValues.begin(); iter != newValues.end(); ++iter) {
        std::stringstream ss;
        ss << streamJoin(iter->second).delimiter(delim).emptyString(".");
        rv.set(iter->first, ss.str());
    }

    return rv;
}


}

END_NAMESPACE(Vcf)
