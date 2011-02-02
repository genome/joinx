#pragma once

#include "common/intconfig.hpp"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <map>
#include <string>

class CommandBase {
public:
    typedef boost::shared_ptr<CommandBase> ptr;

    virtual ~CommandBase() {}

    virtual ptr create(int argc, char** argv) = 0;
    virtual void exec() = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;

    ptr subCommand(const std::string& name, int argc, char** argv) const;
    void registerSubCommand(const ptr& app);
    void describeSubCommands(std::ostream& s, const std::string& indent = "\t");

protected:
    typedef std::map<std::string, ptr> SubCommandMap;
    SubCommandMap _subCmds;
};
