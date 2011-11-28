#pragma once

#include <set>
#include <vector>
#include <functional>

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

template<typename ValueType, typename StreamPtr, typename OutputFunc>
class MergeSorted {
public:
    MergeSorted(const std::vector<StreamPtr>& sortedInputs, OutputFunc& output)
        : _sortedInputs(&streamLessThan<StreamPtr>)
        , _output(output)
    {
        for (auto i = sortedInputs.begin(); i != sortedInputs.end(); ++i)
            if (!(*i)->eof())
                _sortedInputs.insert(*i);
    }

    virtual ~MergeSorted() {}

    bool nextSorted(ValueType& next) {
        using namespace std;
        if (_sortedInputs.empty())
            return false;

        bool rv = false;
        while (rv == false && !_sortedInputs.empty()) {
            StreamPtr s = *_sortedInputs.begin();
            _sortedInputs.erase(_sortedInputs.begin());
            if ((rv = s->next(next))) {
                ValueType* p;
                if (s->peek(&p))
                    _sortedInputs.insert(s);
            }
        }

        return rv;
    }

    void execute() {
        ValueType v;
        while (nextSorted(v))
            _output(v);
    }

protected:
    typedef bool (*Compare)(const StreamPtr&, const StreamPtr&);
    std::multiset<StreamPtr, Compare> _sortedInputs;
    OutputFunc& _output;
};
