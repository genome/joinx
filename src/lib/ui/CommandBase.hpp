#pragma once

#include "common/Exceptions.hpp"
#include "common/cstdint.hpp"
#include "fileformats/StreamHandler.hpp"

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <string>

class CommandBase {
public:
    typedef boost::shared_ptr<CommandBase> ptr;

    virtual ~CommandBase() {}

    virtual ptr create(int argc, char** argv) = 0;
    virtual void exec() = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual bool hidden() const {
        return false;
    }

    ptr subCommand(const std::string& name, int argc, char** argv) const;
    void registerSubCommand(const ptr& app);
    void describeSubCommands(std::ostream& s, const std::string& indent = "\t");

protected:
    typedef std::map<std::string, ptr> SubCommandMap;
    SubCommandMap _subCmds;
};

namespace NewCommands {

class CommandBase {
public:
    typedef boost::shared_ptr<CommandBase> ptr;

    CommandBase();
    virtual ~CommandBase() {}

    virtual void exec() = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual std::vector<std::string> const& requiredOptions() const = 0;

    virtual void configureOptions() = 0;
    virtual void parseCommandLine(int argc, char** argv);

    virtual bool hidden() const {
        return false;
    }


protected:
    StreamHandler _streams;
    boost::program_options::options_description _opts;
    boost::program_options::positional_options_description _posOpts;
};

}
