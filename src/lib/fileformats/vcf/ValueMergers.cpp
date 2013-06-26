#include "ValueMergers.hpp"

#include "Entry.hpp"
#include "CustomType.hpp"
#include "CustomValue.hpp"

#include <boost/format.hpp>
#include <set>
#include <stdexcept>

using boost::format;
using namespace std;

BEGIN_NAMESPACE(Vcf)

namespace ValueMergers {

std::unique_ptr<Registry> Registry::_instance;

Registry::Registry() {
    registerMerger(Base::const_ptr(new UseFirst));
    registerMerger(Base::const_ptr(new EnforceEquality));
    registerMerger(Base::const_ptr(new Ignore));
    registerMerger(Base::const_ptr(new Sum));
    registerMerger(Base::const_ptr(new UniqueConcat));
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
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end
    ) const
{
    const CustomValue* v(fetch(begin));
    if (!v)
        return CustomValue();
    return *v;
}

CustomValue UniqueConcat::operator()(
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end
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
        for (const Entry* e = begin; e != end; ++e) {
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
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end
    ) const
{
    CustomValue rv;
    for (const Entry* e = begin; e != end; ++e) {
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
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end
    ) const
{
    CustomValue rv(type);
    for (const Entry* e = begin; e != end; ++e) {
        const CustomValue* v = fetch(e);
        if (!v || v->empty())
            continue;
        rv += *v;
    }
    return rv;
}

CustomValue Ignore::operator()(
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end
    ) const
{
    return CustomValue();
}

CustomValue PerAltDelimitedList::operator()(
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end
    ) const
{
    if (type->numberType() != CustomType::PER_ALLELE) {
        throw runtime_error(str(format(
            "%1% merge strategy used with something other than per-allele type (%2%)"
            ) % name() % CustomType::numberToString(type->numberType(), type->number())));
    }
    CustomValue rv(type);
    for (Entry const* e = begin; e != end; ++e) {
        CustomValue const* v = fetch(e);
        if (!v || v->empty())
            continue;
    }
    return rv;
}


}

END_NAMESPACE(Vcf)
