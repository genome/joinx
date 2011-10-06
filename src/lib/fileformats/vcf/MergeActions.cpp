#include "MergeActions.hpp"

#include "Entry.hpp"
#include "CustomType.hpp"
#include "CustomValue.hpp"

#include <boost/format.hpp>
#include <set>

using boost::format;
using namespace std;

VCF_NAMESPACE_BEGIN

namespace MergeActions {

CustomValue UniqueConcat::operator()(
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end)
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

CustomValue Equality::operator()(
    const CustomType* type,
    FetchFunc fetch,
    const Entry* begin,
    const Entry* end
    )
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
    )
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
    )
{
    return CustomValue();
}

}

VCF_NAMESPACE_END
