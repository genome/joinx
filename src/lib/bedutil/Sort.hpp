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

template<typename StreamFactoryType>
class Sort {
public:
    typedef typename StreamFactoryType::StreamPtr StreamPtr;
    typedef typename StreamFactoryType::ValueType ValueType;
    typedef SortBuffer<StreamFactoryType> BufferType;
    typedef std::shared_ptr<BufferType> BufferPtr;

    Sort(
            StreamFactoryType& streamFactory,
            std::vector<InputStream::ptr> inputs,
            std::ostream& output,
            unsigned maxInMem,
            bool stable,
            CompressionType compression = NONE
        )
        : _streamFactory(streamFactory)
        , _output(output)
        , _maxInMem(maxInMem)
        , _stable(stable)
        , _compression(compression)
    {
        for (auto iter = inputs.begin(); iter != inputs.end(); ++iter)
            _inputs.push_back(streamFactory.open(**iter));
    }

    void execute() {
        using namespace std;

        BufferPtr buf(new BufferType(_streamFactory, _stable, _compression));

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
                    buf.reset(new BufferType(_streamFactory, _stable, _compression));
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
    StreamFactoryType& _streamFactory;
    std::vector<BufferPtr> _buffers;
    std::vector<StreamPtr> _inputs;
    std::ostream& _output;
    unsigned _maxInMem;
    bool _stable;
    CompressionType _compression;
};
