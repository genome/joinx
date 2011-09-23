#pragma once

#include "MergeSorted.hpp"
#include "SortBuffer.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

template<typename StreamFactoryType, typename OutputFunc>
class Sort {
public:
    typedef typename StreamFactoryType::StreamPtr StreamPtr;
    typedef typename StreamFactoryType::ValueType ValueType;
    typedef SortBuffer<StreamFactoryType, OutputFunc> BufferType;
    typedef std::shared_ptr<BufferType> BufferPtr;

    Sort(
            StreamFactoryType& streamFactory,
            std::vector<InputStream::ptr> inputs,
            OutputFunc& out,
            unsigned maxInMem,
            bool stable,
            CompressionType compression = NONE
        )
        : _streamFactory(streamFactory)
        , _out(out)
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
            buf->write(_out);
        } else {
            if (!buf->empty()) {
                buf->sort();
                _buffers.push_back(buf);
            }
            MergeSorted<ValueType, BufferPtr, OutputFunc> merger(_buffers, _out);
            merger.execute();
        }
    }

protected:
    StreamFactoryType& _streamFactory;
    std::vector<BufferPtr> _buffers;
    std::vector<StreamPtr> _inputs;
    OutputFunc& _out;
    unsigned _maxInMem;
    bool _stable;
    CompressionType _compression;
};
