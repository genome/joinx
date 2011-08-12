#include "CommandBase.hpp"

#include <boost/format.hpp>

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
