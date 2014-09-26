#pragma once

#include "MergeSorted.hpp"
#include "SortBuffer.hpp"
#include "common/compat.hpp"
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
    typedef typename std::unique_ptr<StreamType> StreamPtr;
    typedef typename StreamType::ValueType ValueType;
    typedef typename ValueType::HeaderType HeaderType;
    typedef SortBuffer<StreamType, StreamOpener, OutputFunc> BufferType;
    typedef std::unique_ptr<BufferType> BufferPtr;
    typedef std::unique_ptr<Sort> ptr;

    Sort(Sort const&) = delete;
    Sort& operator=(Sort const&) = delete;

    Sort(
            std::vector<StreamPtr> const& inputs,
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

        std::unique_ptr<BufferType> buf(new BufferType(_streamOpener, _outputHeader, _stable, _compression));

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
                    _buffers.push_back(std::move(buf));
                    buf.reset(new BufferType(_streamOpener, _outputHeader,
                        _stable, _compression));
                }
            }
        }

        if (_buffers.empty()) {
            buf->sort();
            buf->write(_out);
        } else {
            if (!buf->empty()) {
                buf->sort();
                _buffers.push_back(std::move(buf));
            }
            auto merger = makeMergeSorted(_buffers);
            ValueType e;
            while (merger.next(e)) {
                _out(e);
            }
        }
    }

protected:
    std::vector<StreamPtr> const& _inputs;
    StreamOpener& _streamOpener;
    OutputFunc& _out;
    HeaderType& _outputHeader;
    std::vector<BufferPtr> _buffers;
    uint64_t _maxInMem;
    bool _stable;
    CompressionType _compression;
};

template<typename StreamType, typename StreamOpener, typename OutputFunc>
typename Sort<StreamType, StreamOpener, OutputFunc>::ptr makeSort(
          std::vector<std::unique_ptr<StreamType>> const& inputs
        , StreamOpener& streamOpener
        , OutputFunc& out
        , typename StreamType::HeaderType& outputHeader
        , uint64_t maxInMem
        , bool stable
        , CompressionType compression = NONE
        )
{
    return std::make_unique<Sort<StreamType, StreamOpener, OutputFunc>>(
          inputs
        , streamOpener
        , out
        , outputHeader
        , maxInMem
        , stable
        , compression
        );
}
