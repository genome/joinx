#include "JoinX.hpp"

#include <boost/program_options.hpp>
#include <iostream>
#include <stdexcept>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv) {
    try {
        JoinX app(argc, argv);
        app.exec();
    } catch (const po::multiple_occurrences& e) {
        // this doesn't work on boost 1.40, which is what we are stuck with
        // for now :(
//        cerr << "Error: multiple occurrences of option " << e.get_option_name()
//            << ". Abort." << endl;
        cerr << "Multiple occurrences of command line parameters found in argument list." << endl;
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
