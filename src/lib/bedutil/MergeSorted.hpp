#pragma once

#include <vector>
#include <iostream>

template<typename ValueType, typename StreamPtr>
class MergeSorted {
public:
    MergeSorted(std::vector<StreamPtr> sortedInputs, std::ostream& output)
        : _sortedInputs(sortedInputs)
        , _output(output)
    {}

    virtual ~MergeSorted() {}

    bool nextSorted(ValueType& next) {
        using namespace std;
        typedef typename vector<StreamPtr>::const_iterator IterType;

        ValueType* peek;
        IterType minIter = _sortedInputs.end();
        for (IterType iter = _sortedInputs.begin(); iter != _sortedInputs.end(); ++iter) {
            if (!(*iter)->eof() && (*iter)->peek(&peek)) {
                if (minIter == _sortedInputs.end() || *peek < next) {
                    minIter = iter;
                    next = *peek;
                }
            }
        }
        if (minIter != _sortedInputs.end()) {
            (*minIter)->next(next);
            return true;
        }

        return false;
    }

    void execute() {
        ValueType v;
        while (nextSorted(v))
            _output << v << "\n";
    }

protected:
    std::vector<StreamPtr> _sortedInputs;
    std::ostream& _output;
};
