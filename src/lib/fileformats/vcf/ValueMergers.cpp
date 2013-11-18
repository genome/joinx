#include "ValueMergers.hpp"

#include "CustomType.hpp"
#include "CustomValue.hpp"
#include "Entry.hpp"
#include "common/Tokenizer.hpp"
#include "io/StreamJoin.hpp"

#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>

#include <iterator>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)

namespace ValueMergers {

boost::scoped_ptr<Registry> Registry::_instance;

Registry::Registry() {
    registerMerger(Base::const_ptr(new UseFirst));
    registerMerger(Base::const_ptr(new EnforceEquality));
    registerMerger(Base::const_ptr(new Ignore));
    registerMerger(Base::const_ptr(new Sum));
    registerMerger(Base::const_ptr(new UniqueConcat));
    registerMerger(Base::const_ptr(new PerAltDelimitedList));
}

void Registry::registerMerger(Base::const_ptr const& merger) {
    auto inserted = _mergers.insert(make_pair(merger->name(), merger));
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
        _instance.reset(new Registry);
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
    boost::unordered_map<size_t, std::unordered_set<std::string>> newValues;

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
