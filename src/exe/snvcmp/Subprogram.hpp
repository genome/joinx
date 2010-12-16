#pragma once

#include <deque>
#include <string>

class Subprogram {
public:
    virtual ~Subprogram() {}

    virtual std::string name() const = 0;
    virtual void exec(const std::deque<std::string>& args) = 0;
};
