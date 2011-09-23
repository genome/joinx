#pragma once

#include "namespace.hpp"

#include <functional>

VCF_NAMESPACE_BEGIN

class CustomType;
class CustomValue;
class Entry;

namespace MergeActions {

    struct Base {
        typedef std::function<const CustomValue*(const Entry*)> FetchFunc;
        virtual ~Base() {};
        virtual CustomValue operator()(
            const CustomType* type,
            FetchFunc fetch,
            const Entry* begin,
            const Entry* end
            ) = 0;
    };

    struct Concatenate : public Base {
        CustomValue operator()(
            const CustomType* type,
            FetchFunc fetch,
            const Entry* begin,
            const Entry* end
            );
    };

    struct Equality : public Base {
        CustomValue operator()(
            const CustomType* type,
            FetchFunc fetch,
            const Entry* begin,
            const Entry* end
            );
    };
}

VCF_NAMESPACE_END
