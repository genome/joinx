#pragma once

#include <iostream>
#include <vector>
#include <string>

class SortBed {
public:
    SortBed(std::istream& in, std::ostream& out);

    void exec();

protected:
    std::vector<std::ostream*> _tmpfiles;
    std::istream& _in;
    std::ostream& _out;
};
