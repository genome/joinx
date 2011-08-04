#pragma once

#include "CommandBase.hpp"

#include <cstdint>
#include <string>

class JoinX : public CommandBase {
public:
    using CommandBase::ptr;

    JoinX(int argc, char** argv);

    ptr create(int argc, char** argv);
    void exec();
    std::string name() const { return "joinx"; }
    std::string description() const { return "joinx"; }

protected:
    CommandBase::ptr _cmd;
};
