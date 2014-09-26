#include "CommandBase.hpp"

#include "common/compat.hpp"

#include <boost/format.hpp>

#include <sstream>
#include <stdexcept>

#include <iostream>

namespace po = boost::program_options;
using boost::format;
using namespace std;

CommandBase::CommandBase()
    : _optionsParsed(false)
{
}

void CommandBase::parseCommandLine(std::vector<std::string> const& args) {
    if (_optionsParsed) {
        throw OptionsReparsedException(str(format(
            "Attempted to parse command line options multiple times in "
            "command %1%!")
            % name()));
    }

    _opts.add_options()
        ("help,h", "this message")
        ;

    configureOptions();

    try {

        auto parsedArgs = po::command_line_parser(args)
                .options(_opts)
                .positional(_posOpts).run();

        po::store(parsedArgs, _varMap);

        _parsedArgs = std::make_unique<po::parsed_options>(parsedArgs);

        po::notify(_varMap);

    } catch (...) {
        // program options will throw if required options are not passed
        // before we have a chance to check if the user has asked for
        // --help. If they have, let's give it to them, otherwise, rethrow.
        checkHelp();
        throw;
    }

    checkHelp();
    finalizeOptions();
}

void CommandBase::checkHelp() const {
    if (_varMap.count("help")) {
        stringstream ss;
        ss << "Command: " << name() << "\n\n"
            << "Description:\n" << description() << "\n\n"
            << "Options:\n" << _opts;
        throw CmdlineHelpException(ss.str());
    }
}

void CommandBase::parseCommandLine(int argc, char** argv) {
    assert(argc > 0);
    std::vector<std::string> args(argv + 1, argv + argc);
    parseCommandLine(args);
}
