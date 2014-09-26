#pragma once

#include "common/LocusCompare.hpp"
#include "io/TempFile.hpp"
#include "io/InputStream.hpp"
#include "fileformats/StreamFactory.hpp"

#include <boost/format.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <algorithm>
#include <deque>
#include <iterator>
#include <memory>
#include <stdexcept>

template<
          typename StreamType
        , typename StreamOpener
        , typename OutputFunc
        , typename LessThanCmp = CompareToLessThan<LocusCompare<>>
        >
class SortBuffer {
public:
    typedef typename std::unique_ptr<StreamType> StreamPtr;
    typedef typename StreamType::ValueType ValueType;
    typedef typename ValueType::HeaderType HeaderType;
    typedef typename std::deque<ValueType*>::size_type size_type;

    SortBuffer(
              StreamOpener& streamOpener
            , const HeaderType& h
            , bool stable
            , CompressionType compression
            , LessThanCmp cmp = LessThanCmp()
            )
        : _streamOpener(streamOpener)
        , _header(h)
        , _stable(stable)
        , _compression(compression)
        , _inputStream("anon", _in)
        , _cmp(cmp)
    {}

    ~SortBuffer() {
        for (auto iter = _buf.begin(); iter != _buf.end(); ++iter)
            delete *iter;
    }

    void push_back(ValueType* value) {
        _buf.push_back(value);
    }

    void sort() {
        if (_stable)
            std::stable_sort(_buf.begin(), _buf.end(), _cmp);
        else
            std::sort(_buf.begin(), _buf.end(), _cmp);
    }

    size_type size() const {
        return _buf.size();
    }

    bool empty() const {
        return _buf.empty() && _stream.get() == NULL;
    }

    void write(OutputFunc& out) const {
        for (auto iter = _buf.begin(); iter != _buf.end(); ++iter)
            out(**iter);
    }

    void writeTmp() {
        namespace io = boost::iostreams;

        if (_tmpfile.get() != NULL)
            throw std::runtime_error("Attempt to re-serialize sort buffer");

        _tmpfile = TempFile::create(TempFile::ANON);

        // data won't be flushed until filtering_stream goes out of scope
        {
            io::filtering_stream<io::output> out;
            io::gzip_compressor gzip;

            switch (_compression) {
                case GZIP: out.push(gzip); break;
                case NONE:
                default:
                    break;
            }

            out.push(_tmpfile->stream());
            out << _header;
            for (auto iter = _buf.begin(); iter != _buf.end(); ++iter) {
                out << **iter << "\n";
                delete *iter;
            }
            _buf.clear();
        }

        _tmpfile->stream().seekg(0);

        switch (_compression) {
            case GZIP: _in.push(_gzipDecompressor); break;
            case NONE:
            default:
                break;
        }

        _in.push(_tmpfile->stream());

        _stream = _streamOpener(_inputStream);
    }

    bool peek(ValueType** v) {
        if (_stream.get() != NULL)
            return _stream->peek(v);

        if (_buf.empty())
            return false;

        *v = _buf[0];
        return true;
    }

    bool next(ValueType& v) {
        if (_stream.get() != NULL)
            return _stream->next(v);

        if (_buf.empty())
            return false;

        v = *_buf[0];
        delete _buf[0];
        _buf.pop_front();
        return true;
    }

    bool eof() const {
        if (_stream.get() != NULL)
            return _stream->eof();

        return _buf.empty();
    }

protected:
    StreamOpener& _streamOpener;
    const HeaderType& _header;
    bool _stable;
    CompressionType _compression;
    std::deque<ValueType*> _buf;
    TempFile::ptr _tmpfile;
    StreamPtr _stream;

    // for reading compressed tmp file
    boost::iostreams::filtering_stream<boost::iostreams::input> _in;
    boost::iostreams::gzip_decompressor _gzipDecompressor;
    InputStream _inputStream;
    LessThanCmp _cmp;
};
