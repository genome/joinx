#include "Sort.hpp"
#include "fileformats/BedStream.hpp"
#include "bedutil/SortBed.hpp"

#include <fstream>
#include <stdexcept>

using namespace std;

string Sort::name() const {
    return "sort";
}

void Sort::exec(const deque<string>& args) {
    if (args.size() < 2)
        throw runtime_error(name() + " expects 2 positional arguments");

    string fileIn = args[0];
    string fileOut = args[1];

    ifstream in(fileIn.c_str());
    if (!in) throw runtime_error("Failed to open input file " + fileIn);
    ofstream out(fileOut.c_str());
    if (!out) throw runtime_error("Failed to open ouput file " + fileOut);
    BedStream inp(fileIn, in);
    SortBed sorter(inp, out, 1000000);
    sorter.exec();
}
