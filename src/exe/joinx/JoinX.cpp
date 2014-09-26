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
#include "ui/VcfCompareGtCommand.hpp"
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

#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>

#include <memory>
#include <sstream>
#include <stdexcept>


using boost::format;
using namespace std;

JoinX::JoinX() {
    registerSubCommand(std::make_unique<BedMergeCommand>());
    registerSubCommand(std::make_unique<CheckRefCommand>());
    registerSubCommand(std::make_unique<CreateContigsCommand>());
    registerSubCommand(std::make_unique<FindHomopolymersCommand>());
    registerSubCommand(std::make_unique<GenerateCommand>());
    registerSubCommand(std::make_unique<IntersectCommand>());
    registerSubCommand(std::make_unique<RefStatsCommand>());
    registerSubCommand(std::make_unique<RemapCigarCommand>());
    registerSubCommand(std::make_unique<SortCommand>());
    registerSubCommand(std::make_unique<Vcf2RawCommand>());
    registerSubCommand(std::make_unique<VcfAnnotateCommand>());
    registerSubCommand(std::make_unique<VcfAnnotateHomopolymersCommand>());
    registerSubCommand(std::make_unique<VcfCompareCommand>());
    registerSubCommand(std::make_unique<VcfCompareGtCommand>());
    registerSubCommand(std::make_unique<VcfFilterCommand>());
    registerSubCommand(std::make_unique<VcfSiteFilterCommand>());
    registerSubCommand(std::make_unique<VcfMergeCommand>());
    registerSubCommand(std::make_unique<VcfNormalizeIndelsCommand>());
    registerSubCommand(std::make_unique<VcfReportCommand>());
    registerSubCommand(std::make_unique<VcfRemoveFilteredGtCommand>());
    registerSubCommand(std::make_unique<Wig2BedCommand>());
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

void JoinX::registerSubCommand(CommandBase::ptr app) {
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


