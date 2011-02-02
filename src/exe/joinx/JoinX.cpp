#include "JoinX.hpp"
#include "IntersectApp.hpp"

#include <boost/format.hpp>
#include <sstream>
#include <stdexcept>

using boost::format;
using namespace std;

JoinX::JoinX(int argc, char** argv)
{
    registerSubCommand(IntersectApp::ptr(new IntersectApp));

    stringstream cmdHelp;
    cmdHelp << "Valid subcommands:" << endl << endl;
    describeSubCommands(cmdHelp, "\t");

    if (argc < 2)
        throw runtime_error(str(format("No subcommand specified. %1%") %cmdHelp.str()));

    string cmdstr = argv[1];
    if (cmdstr == "-h" || cmdstr == "--help") {
        throw runtime_error(cmdHelp.str());
    }

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
