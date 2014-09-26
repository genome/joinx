#pragma once

#include "ui/CommandBase.hpp"

#include <memory> // for shared_ptr, old gcc is bad :|

#include <string>

class JoinX {
public:

    JoinX();

    void exec(int argc, char** argv);

    std::string name() const { return "joinx"; }
    std::string description() const { return "joinx"; }

    std::shared_ptr<CommandBase> subCommand(const std::string& name, int argc, char** argv) const;
    void registerSubCommand(std::shared_ptr<CommandBase> app);
    void describeSubCommands(std::ostream& s, const std::string& indent = "\t");

protected:
    typedef std::map<std::string, std::shared_ptr<CommandBase>> SubCommandMap;
    SubCommandMap _subCmds;
};
