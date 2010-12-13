cmake_minimum_required(VERSION 2.6)

project(varicmp)
add_definitions(-Wall -ansi)

include(macros.cmake)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# main project
include_directories("src")
add_subdirectory(src build/src)

# unit tests
find_package(GTest)
if(GTEST_FOUND)
    message("Google Test framework found, building unit tests")
    enable_testing(true)
    add_subdirectory(test build/test)
else(GTEST_FOUND)
    message("Google Test framework not found, no tests will be built")
    message("GTest is available at http://code.google.com/p/googletest/")
    message("Ubuntu users can likely sudo apt-get install gtest")
endif(GTEST_FOUND)

# .deb packaging
set(ARCH "i686")
if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(ARCH "x86_64")
endif(${CMAKE_SIZEOF_VOID_P} MATCHES 8)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tool for comparing variation files")
set(CPACK_PACKAGE_NAME "varicmp")
set(CPACK_PACKAGE_VENDOR "wugsc")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Travis Abbott <tabbott@genome.wustl.edu>")
set(CPACK_SYSTEM_NAME "Linux-${ARCH}")
set(CPACK_TOPLEVEL_TAG "Linux-${ARCH}")
include(CPack)


