cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(TestHelper)

project(snpcmp)
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -ansi)
endif ()

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# main project
include(VersionHelper)
#set(VERSION_MAJOR "2")
#set(VERSION_MINOR "1")
#set(VERSION_PATCH "0")
#set(PROJ_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

include_directories("src")
add_subdirectory(src build/src)

# unit tests
find_package(GTest)
if(GTEST_FOUND)
    message("Google Test framework found, building unit tests")
    enable_testing(true)
    add_subdirectory(test build/test)
else()
    message("Google Test framework not found, no tests will be built")
    message("GTest is available at http://code.google.com/p/googletest/")
    message("Ubuntu users can likely sudo apt-get install gtest")
endif()

# .deb packaging
set(ARCH "i686")
if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(ARCH "x86_64")
endif ()

set(CPACK_PACKAGE_INSTALLATION_DIRECTORY "snpcmp-${PROJ_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tool for comparing snp bed files")
set(CPACK_PACKAGE_NAME "snpcmp")
set(CPACK_PACKAGE_VENDOR "wugsc")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJ_VERSION})
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Travis Abbott <tabbott@genome.wustl.edu>")
set(CPACK_SYSTEM_NAME "Linux-${ARCH}")
set(CPACK_TOPLEVEL_TAG "Linux-${ARCH}")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst;")
include(CPack)
