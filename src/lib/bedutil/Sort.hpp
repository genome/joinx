#pragma once

#include "MergeSorted.hpp"
#include "SortBuffer.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

template<typename StreamType, typename StreamPtr = StreamType*>
class Sort {
public:
    typedef std::shared_ptr<StreamType> StreamSPtr;
    typedef typename StreamType::ValueType ValueType;
    typedef SortBuffer<StreamType> BufferType;
    typedef std::shared_ptr<BufferType> BufferPtr;

    Sort(std::vector<StreamPtr> inputs, std::ostream& output, unsigned maxInMem, bool stable, CompressionType compression = NONE)
        : _inputs(inputs)
        , _output(output)
        , _maxInMem(maxInMem)
        , _stable(stable)
        , _compression(compression)
    {
    }

    void execute() {
        using namespace std;

        BufferPtr buf(new BufferType(_stable, _compression));

        for (unsigned idx = 0; idx < _inputs.size(); ++idx) {
            
            while (!_inputs[idx]->eof()) {
                ValueType* vptr = new ValueType;
                if (!_inputs[idx]->next(*vptr)) {
                    delete vptr;
                    break;
                }
                buf->push_back(vptr);
                if (buf->size() >= _maxInMem) {
                    buf->sort();
                    buf->writeTmp();
                    _buffers.push_back(buf);
                    buf.reset(new BufferType(_stable, _compression));
                }
            }
        }

        if (_buffers.empty()) {
            buf->sort();
            buf->write(_output);
        } else {
            if (!buf->empty()) {
                buf->sort();
                _buffers.push_back(buf);
            }
            MergeSorted<ValueType, BufferPtr> merger(_buffers, _output);
            merger.execute();
        }
    }

protected:
    std::vector<BufferPtr> _buffers;
    std::vector<StreamPtr> _inputs;
    std::ostream& _output;
    unsigned _maxInMem;
    bool _stable;
    CompressionType _compression;
};
