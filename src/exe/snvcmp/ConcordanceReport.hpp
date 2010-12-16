#pragma once

#include "Subprogram.hpp"

#include <deque>
#include <string>

class ConcordanceReport : public Subprogram {
public:
    std::string name() const;
    void exec(const std::deque<std::string>& args);

protected:
};

