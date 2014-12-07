cmake_minimum_required(VERSION 2.8)

include(ExternalProject)

function(build_boost BOOST_URL BUILD_DIR)
    set(REQUIRED_BOOST_LIBS ${ARGN})

    set(BOOST_ROOT ${BUILD_DIR}/boost)
    set(BOOST_SRC ${BUILD_DIR}/boost-src)
    set(BOOST_BUILD_LOG ${BOOST_SRC}/build.log)

    foreach(libname ${REQUIRED_BOOST_LIBS})
        set(BOOST_BUILD_LIBS ${BOOST_BUILD_LIBS} --with-${libname})
        set(BOOST_LIBS ${BOOST_LIBS}
            ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_${libname}${CMAKE_STATIC_LIBRARY_SUFFIX}
            )
    endforeach(libname ${REQUIRED_BOOST_LIBS})
    set(Boost_LIBRARIES ${BOOST_LIBS} PARENT_SCOPE)

    message(STATUS "Boost build log will be written to ${BOOST_BUILD_LOG}")
    ExternalProject_Add(
        boost-1.56
        URL ${BOOST_URL}
        SOURCE_DIR ${BOOST_SRC}
        BINARY_DIR ${BOOST_SRC}
        CONFIGURE_COMMAND "./bootstrap.sh"
        BUILD_COMMAND
            ./b2 -d+2 --prefix=${BOOST_ROOT} --layout=system link=static threading=multi install
                cxxflags=${CMAKE_CXX_FLAGS}
                ${BOOST_BUILD_LIBS} ${BOOST_BUILD_OPTS} > ${BOOST_BUILD_LOG}
        INSTALL_COMMAND ""
    )

    add_dependencies(deps boost-1.56)
    set(Boost_INCLUDE_DIRS ${BOOST_ROOT}/include PARENT_SCOPE)

endfunction(build_boost BOOST_VERSION BUILD_DIR)

function(build_zlib ZLIB_URL BUILD_DIR)
    set(ZLIB_ROOT ${CMAKE_BINARY_DIR}/vendor/zlib)
    set(ZLIB_SRC ${CMAKE_BINARY_DIR}/vendor/zlib-src)
    ExternalProject_Add(
        zlib-1.2.8
        URL ${ZLIB_URL}
        SOURCE_DIR ${ZLIB_SRC}
        BINARY_DIR ${ZLIB_SRC}
        CONFIGURE_COMMAND ./configure --prefix=${ZLIB_ROOT}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
    )
    set(ZLIB_INCLUDE_DIRS ${ZLIB_ROOT}/include PARENT_SCOPE)
    set(ZLIB_LIBRARIES ${ZLIB_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}z${CMAKE_STATIC_LIBRARY_SUFFIX}
        PARENT_SCOPE)

    add_library(z STATIC IMPORTED)
    set_property(TARGET z PROPERTY IMPORTED_LOCATION ${ZLIB_LIBRARIES})

    add_dependencies(deps zlib-1.2.8)
endfunction(build_zlib ZLIB_URL BUILD_DIR)
