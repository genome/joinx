#pragma once

#include <fstream>
#include <vector>
#include <string>

#include <set>

class Bed;
class BedStream;

class SortBed {
public:
    SortBed(BedStream& in, std::ostream& out, unsigned maxInMem = 1000000);
    ~SortBed();

    void exec();

protected:
    void createTempFile(const std::multiset<Bed>& beds);
    bool getNextSortedFromFiles(Bed& b);

protected:
    class TempFile;

    std::vector<TempFile*> _tmpfiles;
    BedStream& _in;
    std::ostream& _out;
    unsigned _maxInMem;
};
