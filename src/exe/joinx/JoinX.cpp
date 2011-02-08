#include "version.h"

#include "JoinX.hpp"
#include "IntersectCommand.hpp"
#include "SnvConcordanceCommand.hpp"
#include "SortCommand.hpp"

#include <boost/format.hpp>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

JoinX::JoinX(int argc, char** argv)
{
    registerSubCommand(IntersectCommand::ptr(new IntersectCommand));
    registerSubCommand(SortCommand::ptr(new SortCommand));
    registerSubCommand(SnvConcordanceCommand::ptr(new SnvConcordanceCommand));

    stringstream cmdHelp;
    cmdHelp << "Valid subcommands:" << endl << endl;
    describeSubCommands(cmdHelp, "\t");

    stringstream verInfo;
    verInfo << "joinx version " << __g_prog_version << " (commit " << __g_commit_hash << ")";
 
    if (argc < 2)
        throw runtime_error(str(format("No subcommand specified. %1%") %cmdHelp.str()));

    string cmdstr = argv[1];
    if (cmdstr == "-h" || cmdstr == "--help")
        throw runtime_error(cmdHelp.str());

    if (cmdstr == "-v" || cmdstr == "--version")
        throw runtime_error(verInfo.str());

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
