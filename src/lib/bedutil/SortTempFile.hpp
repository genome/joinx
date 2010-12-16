#pragma once

class SortTempFile {
public:
    SortTempFile();
    ~SortTempFile();

protected:
    std::fstream _f;
};
