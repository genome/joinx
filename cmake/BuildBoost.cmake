cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

set(BOOST_URL http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.gz)

set(BOOST_ROOT ${CMAKE_BINARY_DIR}/vendor/boost)
set(BOOST_SRC ${CMAKE_BINARY_DIR}/vendor/boost-src)

message("Downloading boost from ${BOOST_URL}")
ExternalProject_Add(
    boost-1.54
    URL ${BOOST_URL}
    SOURCE_DIR ${BOOST_SRC}
    BINARY_DIR ${BOOST_SRC}
    CONFIGURE_COMMAND "./bootstrap.sh"
    BUILD_COMMAND
        ./b2 --prefix=${BOOST_ROOT} --layout=system link=static threading=multi install
            --with-program_options --with-filesystem --with-system --with-iostreams
    INSTALL_COMMAND ""
)

set(Boost_INCLUDE_DIRS ${BOOST_ROOT}/include)
set(Boost_LIBRARIES
    ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_program_options${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_filesystem${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_system${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_iostreams${CMAKE_STATIC_LIBRARY_SUFFIX}
)


message("Boost include directory: ${Boost_INCLUDE_DIRS}")
message("Boost libraries: ${Boost_LIBRARIES}")
