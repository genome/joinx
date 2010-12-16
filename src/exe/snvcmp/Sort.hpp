#pragma once

#include "Subprogram.hpp"

#include <deque>
#include <string>

class Sort : public Subprogram {
public:
    std::string name() const;
    void exec(const std::deque<std::string>& args);

protected:
    std::string _fileIn; 
    std::string _fileOut; 
};
