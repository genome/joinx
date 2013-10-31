#include "CommandBase.hpp"

#include <boost/format.hpp>

#include <sstream>
#include <stdexcept>

using namespace std;
using boost::format;

CommandBase::ptr CommandBase::subCommand(const std::string& name, int argc, char** argv) const {
    SubCommandMap::const_iterator iter = _subCmds.find(name);
    if (iter == _subCmds.end())
        return ptr();
    return iter->second->create(argc, argv);
}

void CommandBase::registerSubCommand(const ptr& app) {
    std::pair<SubCommandMap::iterator, bool> result = _subCmds.insert(make_pair(app->name(), app));
    if (!result.second)
        throw runtime_error(str(format("Attempted to register duplicate subcommand name '%1%'") %app->name()));
}

void CommandBase::describeSubCommands(std::ostream& s, const std::string& indent) {
    for (SubCommandMap::const_iterator iter = _subCmds.begin(); iter != _subCmds.end(); ++iter) {
        if (iter->second->hidden())
            continue;
        s << indent << iter->second->name() << " - " << iter->second->description() << endl;
    }
}

namespace NewCommands {

namespace po = boost::program_options;
using boost::format;

CommandBase::CommandBase()
    : _opts("Available Options")
{
}

void CommandBase::parseCommandLine(int argc, char** argv) {
    configureOptions();

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv)
            .options(_opts)
            .positional(_posOpts).run(),
        vm
    );
    po::notify(vm);

    if (vm.count("help")) {
        std::stringstream ss;
        ss << _opts;
        throw std::runtime_error(ss.str());
    }

    auto const& req = requiredOptions();
    for (auto iter = req.begin(); iter != req.end(); ++iter) {
        if (!vm.count(*iter)) {
            throw std::runtime_error(str(format(
                "Required argument '%1%' missing"
                ) % *iter));
        }
    }
}

}
