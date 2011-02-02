#pragma once

#include "MergeSorted.hpp"

#include <boost/filesystem.hpp>
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
    typedef std::vector<ValueType> BufferType;
    typedef boost::shared_ptr<std::fstream> TmpStreamPtr;

    Sort(std::vector<StreamPtr> inputs, std::ostream& output, unsigned maxInMem, bool stable)
        : _inputs(inputs)
        , _output(output)
        , _maxInMem(maxInMem)
        , _stable(stable)
    {}

    virtual ~Sort() {}

    void execute() {
        using namespace std;

        BufferType buf;
        buf.reserve(_maxInMem);

        ValueType val;
        for (unsigned idx = 0; idx < _inputs.size(); ++idx) {
            while (!_inputs[idx]->eof() && _inputs[idx]->next(val)) {
                buf.push_back(val);
                if (buf.size() >= _maxInMem)
                    dumpTempFile(buf);
            }
        }

        if (_tmpFiles.empty()) {
            sortBuffer(buf);
            copy(buf.begin(), buf.end(), ostream_iterator<ValueType>(_output, "\n"));
        } else {
            dumpTempFile(buf);
            vector<boost::shared_ptr<StreamType> > streams;
            for (vector<TmpStreamPtr>::iterator iter = _tmpFiles.begin(); iter != _tmpFiles.end(); ++iter) {
                boost::shared_ptr<StreamType> s(new StreamType("anon", **iter));
                streams.push_back(s);
            }
            MergeSorted<ValueType, boost::shared_ptr<StreamType> > merger(streams, _output);
            merger.execute();
        }
    }

    void dumpTempFile(BufferType& buf) {
        using namespace std;

        // TODO: refactor tmp file creation
        string tmpdir = "/tmp";
        const char* tmpenv = getenv("TMPDIR");
        if (tmpenv != NULL)
            tmpdir = tmpenv;

        string path = tmpdir + "/SortTmp.XXXXXX";
        int fd = mkstemp(&path[0]);
        close(fd); // we just want the name
        TmpStreamPtr stream(new fstream(path.c_str(), ios::in|ios::out|ios::binary));
        boost::filesystem::remove(path); // unlink so file goes away on close
        if (!stream || !stream->is_open())
            throw runtime_error("Failed to open temporary file '" + string(path) + "' while sorting");

        sortBuffer(buf);
        copy(buf.begin(), buf.end(), ostream_iterator<ValueType>(*stream, "\n"));
        _tmpFiles.push_back(stream);

        stream->seekg(0);
        buf.clear();
    }

protected:
    void sortBuffer(BufferType& b) {
        if (_stable)
            stable_sort(b.begin(), b.end());
        else
            sort(b.begin(), b.end());
    }

protected:
    std::vector<StreamPtr> _inputs;
    std::ostream& _output;
    unsigned _maxInMem;
    bool _stable;
    std::vector<TmpStreamPtr> _tmpFiles;
};
