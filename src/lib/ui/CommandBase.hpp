#pragma once

#include "common/Exceptions.hpp"
#include "common/cstdint.hpp"
#include "fileformats/StreamHandler.hpp"

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class OptionsReparsedException : public std::runtime_error {
public:
    OptionsReparsedException(std::string const& msg)
        : std::runtime_error(msg)
    {
    }
};


class CommandBase {
public:
    typedef boost::shared_ptr<CommandBase> ptr;

    CommandBase();
    virtual ~CommandBase() {}

    virtual void exec() = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual bool hidden() const {
        return false;
    }

    void parseCommandLine(int argc, char** argv);
    void parseCommandLine(std::vector<std::string> const& args);

protected:
    virtual void configureOptions() {}
    virtual void finalizeOptions() {}

private:
    void checkHelp() const;

protected:
    bool _optionsParsed;
    boost::program_options::options_description _opts;
    boost::program_options::positional_options_description _posOpts;
    boost::scoped_ptr<boost::program_options::parsed_options> _parsedArgs;
    boost::program_options::variables_map _varMap;
    StreamHandler _streams;
};
