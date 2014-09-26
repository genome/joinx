#pragma once

#include "common/LocusCompare.hpp"
#include "common/RelOps.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace {
    template<typename StreamType, typename LessThanCmp>
    struct StreamLessThan {
        StreamLessThan(LessThanCmp cmp = LessThanCmp())
            : cmp_(cmp)
        {}

        bool operator()(StreamType* a, StreamType* b) const {
            typedef typename StreamType::ValueType ValueType;
            ValueType* pa(0);
            ValueType* pb(0);
            if (a->eof()) return false;
            if (b->eof()) return true;
            a->peek(&pa);
            b->peek(&pb);
            return cmp_(*pa, *pb);
        }

        LessThanCmp cmp_;
    };
}

template<
          typename StreamType
        , typename LessThanCmp = CompareToLessThan<
                typename StreamType::ValueType::DefaultCompare
                >
        >
class MergeSorted {
public:
    typedef typename StreamType::ValueType ValueType;
    typedef std::unique_ptr<StreamType> StreamPtr;
    typedef StreamLessThan<StreamType, LessThanCmp> StreamCmp;

    MergeSorted(std::vector<StreamPtr> inputs, LessThanCmp cmp = LessThanCmp())
        : inputs_(std::move(inputs))
        , sortedInputs_(StreamCmp(cmp))
    {
        for (auto i = inputs_.begin(); i != inputs_.end(); ++i)
            if (!(*i)->eof())
                sortedInputs_.insert(i->get());
    }

    bool next(ValueType& next) {
        using namespace std;
        if (sortedInputs_.empty())
            return false;

        bool rv = false;
        while (rv == false && !sortedInputs_.empty()) {
            StreamType* s = (*sortedInputs_.begin());
            sortedInputs_.erase(sortedInputs_.begin());
            if ((rv = s->next(next))) {
                ValueType* p;
                if (s->peek(&p))
                    sortedInputs_.insert(s);
            }
        }

        return rv;
    }

protected:
    std::vector<StreamPtr> inputs_;
    std::multiset<StreamType*, StreamCmp> sortedInputs_;
};
