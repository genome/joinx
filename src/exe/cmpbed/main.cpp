#include "CmpBedApp.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;

int main(int argc, char** argv) {
    try {
        CmpBedApp app(argc, argv);
        app.exec();
    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}
