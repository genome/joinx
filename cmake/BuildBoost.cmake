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

    ExternalProject_Add(
        boost-1.54
        URL ${BOOST_URL}
        SOURCE_DIR ${BOOST_SRC}
        BINARY_DIR ${BOOST_SRC}
        CONFIGURE_COMMAND "./bootstrap.sh"
        BUILD_COMMAND
            ./b2 --prefix=${BOOST_ROOT} --layout=system link=static threading=multi install
                ${BOOST_BUILD_LIBS} > ${BOOST_BUILD_LOG}
        INSTALL_COMMAND ""
    )

    add_dependencies(deps boost-1.54)
    set(Boost_INCLUDE_DIRS ${BOOST_ROOT}/include PARENT_SCOPE)

endfunction(build_boost BOOST_VERSION BUILD_DIR)

