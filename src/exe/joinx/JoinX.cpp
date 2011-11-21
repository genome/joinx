#include "JoinX.hpp"
#include "CheckRefCommand.hpp"
#include "CreateContigsCommand.hpp"
#include "GenerateCommand.hpp"
#include "IntersectCommand.hpp"
#include "RemapCigarCommand.hpp"
#include "SnvConcordanceByQualityCommand.hpp"
#include "SnvConcordanceCommand.hpp"
#include "SortCommand.hpp"
#include "VcfAnnotateCommand.hpp"
#include "VcfFilterCommand.hpp"
#include "VcfMergeCommand.hpp"

#include "common/ProgramVersion.hpp"

#include <boost/format.hpp>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

JoinX::JoinX(int argc, char** argv)
{
    registerSubCommand(CheckRefCommand::ptr(new CheckRefCommand));
    registerSubCommand(CreateContigsCommand::ptr(new CreateContigsCommand));
    registerSubCommand(GenerateCommand::ptr(new GenerateCommand));
    registerSubCommand(IntersectCommand::ptr(new IntersectCommand));
    registerSubCommand(RemapCigarCommand::ptr(new RemapCigarCommand));
    registerSubCommand(SnvConcordanceByQualityCommand::ptr(new SnvConcordanceByQualityCommand));
    registerSubCommand(SnvConcordanceCommand::ptr(new SnvConcordanceCommand));
    registerSubCommand(SortCommand::ptr(new SortCommand));
    registerSubCommand(VcfAnnotateCommand::ptr(new VcfAnnotateCommand));
    registerSubCommand(VcfFilterCommand::ptr(new VcfFilterCommand));
    registerSubCommand(VcfMergeCommand::ptr(new VcfMergeCommand));

    stringstream cmdHelp;
    cmdHelp << "Valid subcommands:" << endl << endl;
    describeSubCommands(cmdHelp, "\t");


    if (argc < 2)
        throw runtime_error(str(format("No subcommand specified. %1%") %cmdHelp.str()));

    string cmdstr = argv[1];
    if (cmdstr == "-h" || cmdstr == "--help")
        throw runtime_error(cmdHelp.str());

    if (cmdstr == "-v" || cmdstr == "--version")
        throw runtime_error(makeProgramVersionInfo("joinx"));

    _cmd = subCommand(cmdstr, argc-1, &argv[1]);
    if (!_cmd)
        throw runtime_error(str(format("Invalid subcommand '%1%'. %2%") %cmdstr %cmdHelp.str()));
}

CommandBase::ptr JoinX::create(int argc, char** argv) {
    return ptr(new JoinX(argc, argv));
}

void JoinX::exec() {
    _cmd->exec();
}
