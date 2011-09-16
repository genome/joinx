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

CustomValue Concatenate::operator()(
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
        seen.insert(v->toString());
        for (const Entry* e = begin; e != end; ++e) {
            const CustomValue *v = fetch(e);
            if (v) {
                auto inserted = seen.insert(v->toString());
                if (inserted.second) {
                    rv.append(*v);
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
            throw runtime_error(str(format("Equality condition failed! %2% vs %3%")
                %rv.toString() %v->toString()));
        }
    }
    return rv;
}

}

VCF_NAMESPACE_END
