cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

function(build_boost BOOST_VERSION BUILD_DIR)
    string(REPLACE . _ BVU ${BOOST_VERSION})

    set(BOOST_URL
        http://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/boost_${BVU}.tar.gz)

    set(BOOST_ROOT ${BUILD_DIR}/boost)
    set(BOOST_SRC ${BUILD_DIR}/boost-src)
    set(BOOST_BUILD_LOG ${BOOST_SRC}/build.log})

    message("Downloading boost from ${BOOST_URL}")
    ExternalProject_Add(
        boost-1.54
        URL ${BOOST_URL}
        SOURCE_DIR ${BOOST_SRC}
        BINARY_DIR ${BOOST_SRC}
        CONFIGURE_COMMAND "./bootstrap.sh"
        BUILD_COMMAND
            ./b2 --prefix=${BOOST_ROOT} --layout=system link=static threading=multi install
                --with-program_options --with-filesystem --with-system --with-iostreams > ${BOOST_BUILD_LOG}
        INSTALL_COMMAND ""
    )

    set(Boost_INCLUDE_DIRS ${BOOST_ROOT}/include PARENT_SCOPE)
    set(Boost_LIBRARIES
        ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_program_options${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_filesystem${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_system${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_iostreams${CMAKE_STATIC_LIBRARY_SUFFIX}
        PARENT_SCOPE
    )


    message("Boost include directory: ${Boost_INCLUDE_DIRS}")
    message("Boost libraries: ${Boost_LIBRARIES}")
endfunction(build_boost BOOST_VERSION BUILD_DIR)
