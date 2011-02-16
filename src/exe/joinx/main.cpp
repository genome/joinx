#include "JoinX.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;

int main(int argc, char** argv) {
    try {
        JoinX app(argc, argv);
        app.exec();
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
