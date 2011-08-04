#pragma once

#include "MergeSorted.hpp"
#include "common/TempFile.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

template<typename StreamType, typename StreamPtr = StreamType*>
class Sort {
public:
    typedef typename StreamType::ValueType ValueType;
    typedef std::vector<ValueType*> BufferType;

    Sort(std::vector<StreamPtr> inputs, std::ostream& output, unsigned maxInMem, bool stable)
        : _inputs(inputs)
        , _output(output)
        , _maxInMem(maxInMem)
        , _stable(stable)
    {
    }

    void execute() {
        using namespace std;

        BufferType buf;
        buf.reserve(_maxInMem);

        for (unsigned idx = 0; idx < _inputs.size(); ++idx) {
            
            while (!_inputs[idx]->eof()) {
                ValueType* vptr = new ValueType;
                if (!_inputs[idx]->next(*vptr)) {
                    delete vptr;
                    break;
                }
                buf.push_back(vptr);
                if (buf.size() >= _maxInMem)
                    dumpTempFile(buf);
            }
        }

        if (_tmpFiles.empty()) {
            sortBuffer(buf);
            for (auto iter = buf.begin(); iter != buf.end(); ++iter)
                _output << **iter << "\n";
        } else {
            dumpTempFile(buf);
            vector<boost::shared_ptr<StreamType> > streams;
            for (vector<TempFile::ptr>::iterator iter = _tmpFiles.begin(); iter != _tmpFiles.end(); ++iter) {
                boost::shared_ptr<StreamType> s(new StreamType("anon", (*iter)->stream()));
                streams.push_back(s);
            }
            MergeSorted<ValueType, boost::shared_ptr<StreamType> > merger(streams, _output);
            merger.execute();
        }

        for (auto iter = buf.begin(); iter != buf.end(); ++iter)
            delete *iter;
    }

    void dumpTempFile(BufferType& buf) {
        if (buf.empty())
            return;

        if (_tmpdir.get() == NULL)
            _tmpdir = TempDir::create(TempDir::CLEANUP);

        TempFile::ptr tmp(_tmpdir->tempFile(TempFile::ANON));

        sortBuffer(buf);
        for (auto iter = buf.begin(); iter != buf.end(); ++iter) {
            tmp->stream() << **iter << "\n";
            delete *iter;
        }
        tmp->stream().seekg(0);
        _tmpFiles.push_back(tmp);
        buf.clear();
    }

protected:

    static int _cmp(const ValueType* a, const ValueType* b) {
        return *a < *b;
    }

    void sortBuffer(BufferType& b) {
        if (_stable)
            stable_sort(b.begin(), b.end(), _cmp);
        else
            sort(b.begin(), b.end(), _cmp);
    }

protected:
    std::vector<StreamPtr> _inputs;
    std::ostream& _output;
    unsigned _maxInMem;
    bool _stable;
    std::vector<TempFile::ptr> _tmpFiles;
    TempDir::ptr _tmpdir;
};
