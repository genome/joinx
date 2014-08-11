#pragma once

#include <set>
#include <vector>
#include <functional>
#include <utility>

namespace {
    template<typename StreamPtr>
    bool streamLessThan(const StreamPtr& a, const StreamPtr& b) {
        typedef typename StreamPtr::element_type::ValueType ValueType;
        ValueType* pa(0);
        ValueType* pb(0);
        if (a->eof()) return false;
        if (b->eof()) return true;
        a->peek(&pa);
        b->peek(&pb);
        return *pa < *pb;
    }
}

template<typename ValueType_, typename StreamPtr>
class MergeSorted {
public:
    typedef ValueType_ ValueType;

    MergeSorted(const std::vector<StreamPtr>& sortedInputs)
        : sortedInputs_(&streamLessThan<StreamPtr>)
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
    typedef bool (*Compare)(const StreamPtr&, const StreamPtr&);
    std::multiset<StreamPtr, Compare> sortedInputs_;
};
