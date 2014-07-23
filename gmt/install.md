Build instructions
------------------

### Setup repository
* Clone the git repository
* Initialize submodules: `git submodule update --init`

### Install build dependencies

Building requires:

* gcc 4.4+ or clang 3.2+
* cmake 2.8+
* zlib development files (headers, libraries)

Integration tests require:

* Python
* Valgrind (optional, but recommended)

To install these on Ubuntu, try:

`sudo apt-get install cmake build-essential zlib1g-dev`

### Build, test, and install

Note: if using clang, export the environment variables CC=clang,
CXX=clang++.

Joinx does not support in source builds. Create a new build directory,
enter it, and run:

-   `cmake /path/to/joinx/repo [opts]`
    -   Some useful options to cmake are:
        -   -DBOOST\_ROOT=/path/to/boost if boost is installed somewhere
            other than the default location
        -   -DBoost\_NO\_SYSTEM\_PATHS=on to skip checking system
            default locations for boost headers and libs
        -   -DBoost\_USE\_STATIC\_LIBS=on to link boost statically
        -   -DCMAKE\_INSTALL\_PREFIX=/path change the installation
            location (default: /usr)
-   `make deps`
-   `make`
-   `ctest` (to run unit tests)
-   `make install`

Running ctest with run both unit and integration tests. you can run one
or the other with `ctest -L <unit|integration>`