#include "BedSortApp.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;

int main(int argc, char** argv) {
    try {
        BedSortApp app(argc, argv);
        app.exec();
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
