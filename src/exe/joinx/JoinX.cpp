#include "JoinX.hpp"

#include "ui/BedMergeCommand.hpp"
#include "ui/CheckRefCommand.hpp"
#include "ui/CreateContigsCommand.hpp"
#include "ui/FindHomopolymersCommand.hpp"
#include "ui/GenerateCommand.hpp"
#include "ui/IntersectCommand.hpp"
#include "ui/RefStatsCommand.hpp"
#include "ui/RemapCigarCommand.hpp"
#include "ui/SnvConcordanceByQualityCommand.hpp"
#include "ui/SnvConcordanceCommand.hpp"
#include "ui/SortCommand.hpp"
#include "ui/Vcf2RawCommand.hpp"
#include "ui/VcfAnnotateCommand.hpp"
#include "ui/VcfAnnotateHomopolymersCommand.hpp"
#include "ui/VcfCompareGtCommand.hpp"
#include "ui/VcfFilterCommand.hpp"
#include "ui/VcfMergeCommand.hpp"
#include "ui/VcfNormalizeIndelsCommand.hpp"
#include "ui/VcfReportCommand.hpp"
#include "ui/VcfRemoveFilteredGtCommand.hpp"
#include "ui/VcfSiteFilterCommand.hpp"
#include "ui/Wig2BedCommand.hpp"

#include "common/Exceptions.hpp"
#include "common/ProgramDetails.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <stdexcept>


namespace {
    using boost::format;
    using namespace std;
}

JoinX::JoinX() {
    registerSubCommand(BedMergeCommand::ptr(new BedMergeCommand));
    registerSubCommand(CheckRefCommand::ptr(new CheckRefCommand));
    registerSubCommand(CreateContigsCommand::ptr(new CreateContigsCommand));
    registerSubCommand(FindHomopolymersCommand::ptr(new FindHomopolymersCommand));
    registerSubCommand(GenerateCommand::ptr(new GenerateCommand));
    registerSubCommand(IntersectCommand::ptr(new IntersectCommand));
    registerSubCommand(RefStatsCommand::ptr(new RefStatsCommand));
    registerSubCommand(RemapCigarCommand::ptr(new RemapCigarCommand));
    registerSubCommand(SnvConcordanceByQualityCommand::ptr(new SnvConcordanceByQualityCommand));
    registerSubCommand(SnvConcordanceCommand::ptr(new SnvConcordanceCommand));
    registerSubCommand(SortCommand::ptr(new SortCommand));
    registerSubCommand(Vcf2RawCommand::ptr(new Vcf2RawCommand));
    registerSubCommand(VcfAnnotateCommand::ptr(new VcfAnnotateCommand));
    registerSubCommand(VcfAnnotateHomopolymersCommand::ptr(new VcfAnnotateHomopolymersCommand));
    registerSubCommand(VcfCompareGtCommand::ptr(new VcfCompareGtCommand));
    registerSubCommand(VcfFilterCommand::ptr(new VcfFilterCommand));
    registerSubCommand(VcfSiteFilterCommand::ptr(new VcfSiteFilterCommand));
    registerSubCommand(VcfMergeCommand::ptr(new VcfMergeCommand));
    registerSubCommand(VcfNormalizeIndelsCommand::ptr(new VcfNormalizeIndelsCommand));
    registerSubCommand(VcfReportCommand::ptr(new VcfReportCommand));
    registerSubCommand(VcfRemoveFilteredGtCommand::ptr(new VcfRemoveFilteredGtCommand));
    registerSubCommand(Wig2BedCommand::ptr(new Wig2BedCommand));
}

void JoinX::exec(int argc, char** argv) {
    stringstream cmdHelp;
    cmdHelp << "Valid subcommands:" << endl << endl;
    describeSubCommands(cmdHelp, "\t");

    if (argc < 2)
        throw runtime_error(str(format("No subcommand specified. %1%") %cmdHelp.str()));

    string cmdstr = argv[1];
    if (cmdstr == "-h" || cmdstr == "--help")
        throw CmdlineHelpException(cmdHelp.str());

    if (cmdstr == "-v" || cmdstr == "--version")
        throw CmdlineHelpException(makeProgramVersionInfo("joinx"));

    auto found = _subCmds.find(cmdstr);
    if (found == _subCmds.end())
        throw runtime_error(str(format("Invalid subcommand '%1%'. %2%") %cmdstr %cmdHelp.str()));

    auto cmd = found->second;
    cmd->parseCommandLine(argc - 1, &argv[1]);
    cmd->exec();
}

CommandBase::ptr JoinX::subCommand(const std::string& name, int argc, char** argv) const {
    SubCommandMap::const_iterator iter = _subCmds.find(name);
    if (iter == _subCmds.end())
        return CommandBase::ptr();
    return iter->second;
}

void JoinX::registerSubCommand(const CommandBase::ptr& app) {
    std::pair<SubCommandMap::iterator, bool> result = _subCmds.insert(make_pair(app->name(), app));
    if (!result.second)
        throw runtime_error(str(format("Attempted to register duplicate subcommand name '%1%'") %app->name()));
}

void JoinX::describeSubCommands(std::ostream& s, const std::string& indent) {
    for (SubCommandMap::const_iterator iter = _subCmds.begin(); iter != _subCmds.end(); ++iter) {
        if (iter->second->hidden())
            continue;
        s << indent << iter->second->name() << " - " << iter->second->description() << endl;
    }
}


