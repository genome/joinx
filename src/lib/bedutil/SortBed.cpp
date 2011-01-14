#include "SortBed.hpp"

#include "fileformats/Bed.hpp"
#include "fileformats/BedStream.hpp"

#include <boost/bind.hpp>

#include <algorithm>
#include <set>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <unistd.h>

using namespace std;

class SortBed::TempFile {
public:
    TempFile(const multiset<Bed>& beds)
    {
        strcpy(_path, "/tmp/sortbed.XXXXXX");
        // TODO: refactor tmp file creation
        int fd = mkstemp(&_path[0]);
        close(fd);
        _f.open(_path, ios::in|ios::out|ios::binary);
        if (!_f.is_open())
            throw runtime_error("Failed to open temporary file '" + string(_path) + "' while sorting");

        typedef multiset<Bed>::const_iterator IterType;
        for (IterType iter = beds.begin(); iter != beds.end(); ++iter)
            _f << iter->line << endl;

        _f.seekg(0);
        _in.reset(new BedStream(_path, _f));
    }

    ~TempFile() {
        unlink(_path);
        _in.reset();
    }

    BedStream& in() {
        return *_in;
    }

    const BedStream& in() const {
        return *_in;
    }

    bool eof() const {
        return in().eof();
    }

    bool operator<(TempFile& rhs) {
        Bed* a = NULL;
        Bed* b = NULL;
        return in().peek(&a) && rhs.in().peek(&b) && *a < *b;
    }

protected:
    char _path[4096];
    fstream _f;
    auto_ptr<BedStream> _in;
};


SortBed::SortBed(BedStream& in, std::ostream& out, unsigned maxInMem /* = 1m */)
    : _in(in)
    , _out(out)
    , _maxInMem(maxInMem)
{
}

SortBed::~SortBed() {
    for (vector<TempFile*>::iterator iter = _tmpfiles.begin(); iter != _tmpfiles.end(); ++iter)
        delete *iter;
}

void SortBed::exec() {
    multiset<Bed> beds;
    Bed bed;
    while (!_in.eof()) {
        _in >> bed;
        beds.insert(bed);
        if (beds.size() > _maxInMem) {
            createTempFile(beds);
            beds.clear();
        }
    }

    if (_tmpfiles.empty()) { 
        for (multiset<Bed>::const_iterator iter = beds.begin(); iter != beds.end(); ++iter)
            _out << iter->line << endl;
    } else {
        if (!beds.empty())
            createTempFile(beds);

        while (getNextSortedFromFiles(bed))
            _out << bed.line << endl;
    }
}

bool SortBed::getNextSortedFromFiles(Bed& bed) {
    if (_tmpfiles.empty())
        return false;

    uint64_t minIdx = 0;
    for (uint64_t i = 0; i < _tmpfiles.size(); ++i) {
        if (i == minIdx || _tmpfiles[i]->eof())
            continue;
        if (_tmpfiles[minIdx]->eof() || *_tmpfiles[i] < *_tmpfiles[minIdx])
            minIdx = i;
    }

    if (!_tmpfiles[minIdx]->eof()) {
        _tmpfiles[minIdx]->in() >> bed;
        return true;
    }

    return false;
}

void SortBed::createTempFile(const multiset<Bed>& beds) {
    _tmpfiles.push_back(new TempFile(beds));
}
