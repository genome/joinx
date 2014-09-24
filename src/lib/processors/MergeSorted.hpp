#pragma once

#include "common/LocusCompare.hpp"

#include <set>
#include <vector>
#include <functional>
#include <utility>

namespace {
    template<typename StreamPtr, typename LessThanCmp>
    struct StreamLessThan {
        StreamLessThan(LessThanCmp cmp = LessThanCmp())
            : cmp_(cmp)
        {}

        bool operator()(const StreamPtr& a, const StreamPtr& b) const {
            typedef typename StreamPtr::element_type::ValueType ValueType;
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
          typename ValueType_
        , typename StreamPtr
        , typename LessThanCmp = CompareToLessThan<LocusCompare<>>
        >
class MergeSorted {
public:
    typedef ValueType_ ValueType;

    MergeSorted(const std::vector<StreamPtr>& sortedInputs, LessThanCmp cmp = LessThanCmp())
        : sortedInputs_(StreamLessThan<StreamPtr, LessThanCmp>(cmp))
        , cmp_(cmp)
    {
        for (auto i = sortedInputs.begin(); i != sortedInputs.end(); ++i)
            if (!(*i)->eof())
                sortedInputs_.insert(*i);
    }

    virtual ~MergeSorted() {}

    bool next(ValueType& next) {
        using namespace std;
        if (sortedInputs_.empty())
            return false;

        bool rv = false;
        while (rv == false && !sortedInputs_.empty()) {
            StreamPtr s = *sortedInputs_.begin();
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
    std::multiset<StreamPtr, StreamLessThan<StreamPtr, LessThanCmp>> sortedInputs_;
    LessThanCmp cmp_;
};
