#include "JoinX.hpp"

#include "ui/BedMergeCommand.hpp"
#include "ui/CheckRefCommand.hpp"
#include "ui/CreateContigsCommand.hpp"
#include "ui/FindHomopolymersCommand.hpp"
#include "ui/GenerateCommand.hpp"
#include "ui/IntersectCommand.hpp"
#include "ui/RefStatsCommand.hpp"
#include "ui/RemapCigarCommand.hpp"
#include "ui/SortCommand.hpp"
#include "ui/Vcf2RawCommand.hpp"
#include "ui/VcfAnnotateCommand.hpp"
#include "ui/VcfAnnotateHomopolymersCommand.hpp"
#include "ui/VcfCompareCommand.hpp"
#include "ui/VcfFilterCommand.hpp"
#include "ui/VcfMergeCommand.hpp"
#include "ui/VcfNormalizeIndelsCommand.hpp"
#include "ui/VcfReportCommand.hpp"
#include "ui/VcfRemoveFilteredGtCommand.hpp"
#include "ui/VcfSiteFilterCommand.hpp"
#include "ui/Wig2BedCommand.hpp"

#include "common/compat.hpp"
#include "common/Exceptions.hpp"
#include "common/ProgramDetails.hpp"

#include <boost/format.hpp>

#include <memory>
#include <sstream>
#include <stdexcept>


using boost::format;
using namespace std;

JoinX::JoinX() {
    registerSubCommand(std::shared_ptr<CommandBase>(new BedMergeCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new CheckRefCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new CreateContigsCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new FindHomopolymersCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new GenerateCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new IntersectCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new RefStatsCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new RemapCigarCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new SortCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new Vcf2RawCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfAnnotateCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfAnnotateHomopolymersCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfCompareCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfFilterCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfSiteFilterCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfMergeCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfNormalizeIndelsCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfReportCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new VcfRemoveFilteredGtCommand));
    registerSubCommand(std::shared_ptr<CommandBase>(new Wig2BedCommand));
}

void JoinX::exec(int argc, char** argv) {
    std::stringstream cmdHelp;
    cmdHelp << "Valid subcommands:" << endl << endl;
    describeSubCommands(cmdHelp, "\t");

    if (argc < 2) {
        throw std::runtime_error(str(format(
            "No subcommand specified. %1%"
            ) % cmdHelp.str()));
    }

    std::string cmdstr = argv[1];
    if (cmdstr == "-h" || cmdstr == "--help")
        throw CmdlineHelpException(cmdHelp.str());

    if (cmdstr == "-v" || cmdstr == "--version")
        throw CmdlineHelpException(makeProgramVersionInfo("joinx"));

    auto found = _subCmds.find(cmdstr);
    if (found == _subCmds.end())
        throw runtime_error(str(format("Invalid subcommand '%1%'. %2%") %cmdstr %cmdHelp.str()));

    auto& cmd = found->second;
    cmd->parseCommandLine(argc - 1, &argv[1]);
    cmd->exec();
}

void JoinX::registerSubCommand(std::shared_ptr<CommandBase> app) {
    auto result = _subCmds.insert(make_pair(app->name(), std::move(app)));
    if (!result.second)
        throw std::runtime_error(str(format(
            "Attempted to register duplicate subcommand name '%1%'"
            ) % app->name()));
}

void JoinX::describeSubCommands(std::ostream& s, const std::string& indent) {
    for (auto iter = _subCmds.begin(); iter != _subCmds.end(); ++iter) {
        if (iter->second->hidden())
            continue;
        s << indent << iter->second->name() << " - "
            << iter->second->description() << "\n";
    }
}


