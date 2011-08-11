#pragma once

#include "common/TempFile.hpp"
#include "fileformats/InputStream.hpp"

#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <algorithm>
#include <deque>
#include <iterator>
#include <memory>
#include <stdexcept>

enum CompressionType {
    NONE,
    ZLIB,
    GZIP,
    BZIP2,
    N_COMPRESSION_TYPES
};

template<typename StreamType>
class SortBuffer {
public:
    typedef typename StreamType::ValueType ValueType;
    typedef typename std::deque<ValueType*>::size_type size_type;

    SortBuffer(bool stable, CompressionType compression)
        : _stable(stable)
        , _compression(compression)
        , _inputStream("anon", _in)
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
            std::stable_sort(_buf.begin(), _buf.end(), cmp);
        else
            std::sort(_buf.begin(), _buf.end(), cmp);
    }

    size_type size() const {
        return _buf.size();
    }

    bool empty() const {
        return _buf.empty() && _stream.get() == NULL;
    }

    void write(std::ostream& s) const {
        for (auto iter = _buf.begin(); iter != _buf.end(); ++iter)
            s << **iter << "\n";
    }

    void writeTmp() {
        namespace io = boost::iostreams;

        if (_tmpfile.get() != NULL)
            throw std::runtime_error("Attempt to re-serialize sort buffer");

        _tmpfile = TempFile::create(TempFile::ANON);

        // data won't be flushed until filtering_stream goes out of scope
        {
            io::filtering_stream<io::output> out;
            io::zlib_compressor zlib;
            io::gzip_compressor gzip;
            io::bzip2_compressor bzip2;

            switch (_compression) {
                case ZLIB: out.push(zlib); break;
                case GZIP: out.push(gzip); break;
                case BZIP2: out.push(bzip2); break;
                case NONE:
                default:
                    break;
            }

            out.push(_tmpfile->stream());
            for (auto iter = _buf.begin(); iter != _buf.end(); ++iter) {
                out << **iter << "\n";
                delete *iter;
            }
            _buf.clear();
        }

        _tmpfile->stream().seekg(0);

        switch (_compression) {
            case ZLIB: _in.push(_zlibDecompressor); break;
            case GZIP: _in.push(_gzipDecompressor); break;
            case BZIP2: _in.push(_bzip2Decompressor); break;
            case NONE:
            default:
                break;
        }

        _in.push(_tmpfile->stream());

        _stream.reset(new StreamType(_inputStream));
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
        _buf.pop_front();
        return true;
    }

    bool eof() const {
        if (_stream.get() != NULL)
            return _stream->eof();

        return _buf.empty();
    }

protected:
    static bool cmp(const ValueType* a, const ValueType* b) {
        return *a < *b;
    }

protected:
    bool _stable;
    CompressionType _compression;
    std::deque<ValueType*> _buf;
    TempFile::ptr _tmpfile;
    std::shared_ptr<StreamType> _stream;

    // for reading compressed tmp file
    boost::iostreams::filtering_stream<boost::iostreams::input> _in;
    boost::iostreams::zlib_decompressor _zlibDecompressor;
    boost::iostreams::gzip_decompressor _gzipDecompressor;
    boost::iostreams::bzip2_decompressor _bzip2Decompressor;
    InputStream _inputStream;
};
