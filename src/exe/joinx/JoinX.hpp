#pragma once

#include "ui/CommandBase.hpp"

#include <string>

class JoinX {
public:

    JoinX();

    void exec(int argc, char** argv);

    std::string name() const { return "joinx"; }
    std::string description() const { return "joinx"; }

    CommandBase::ptr subCommand(const std::string& name, int argc, char** argv) const;
    void registerSubCommand(const CommandBase::ptr& app);
    void describeSubCommands(std::ostream& s, const std::string& indent = "\t");

protected:
    typedef std::map<std::string, CommandBase::ptr> SubCommandMap;
    SubCommandMap _subCmds;
};
