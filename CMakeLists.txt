cmake_minimum_required(VERSION 2.6)

set(CMAKE_INSTALL_PREFIX "/usr")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(TestHelper)
include(VersionHelper)
include(ProjectHelper)

configure_file(debian/postinst.in debian/postinst @ONLY)
configure_file(debian/prerm.in debian/prerm @ONLY)
configure_file(debian/control.in debian/control @ONLY)
configure_file(version/version.h.in version/version.h @ONLY)
include_directories(version)

project(joinx)
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -ansi)
endif ()

find_package(Boost COMPONENTS system program_options filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# main project

include_directories("src/lib")
add_subdirectory(src)

# unit tests
find_package(GTest)
if(GTEST_FOUND)
    message("Google Test framework found, building unit tests")
    enable_testing(true)
    add_projects(test/lib)
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
