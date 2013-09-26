#pragma once

#include "MergeSorted.hpp"
#include "SortBuffer.hpp"
#include "common/cstdint.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

template<typename StreamType, typename StreamOpener, typename OutputFunc>
class Sort {
public:
    typedef typename std::shared_ptr<StreamType> StreamPtr;
    typedef typename StreamType::ValueType ValueType;
    typedef typename ValueType::HeaderType HeaderType;
    typedef SortBuffer<StreamType, StreamOpener, OutputFunc> BufferType;
    typedef std::shared_ptr<BufferType> BufferPtr;

    Sort(
            const std::vector<StreamPtr>& inputs,
            StreamOpener& streamOpener,
            OutputFunc& out,
            HeaderType& outputHeader,
            uint64_t maxInMem,
            bool stable,
            CompressionType compression = NONE
        )
        : _inputs(inputs)
        , _streamOpener(streamOpener)
        , _out(out)
        , _outputHeader(outputHeader)
        , _maxInMem(maxInMem)
        , _stable(stable)
        , _compression(compression)
    {
    }

    void execute() {
        using namespace std;

        BufferPtr buf(new BufferType(_streamOpener, _outputHeader, _stable, _compression));

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
                    buf.reset(new BufferType(_streamOpener, _outputHeader, _stable, _compression));
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
    std::vector<StreamPtr> _inputs;
    StreamOpener& _streamOpener;
    OutputFunc& _out;
    HeaderType& _outputHeader;
    std::vector<BufferPtr> _buffers;
    uint64_t _maxInMem;
    bool _stable;
    CompressionType _compression;
};
