cmake_minimum_required(VERSION 2.8)

project(joinx)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message("No CMAKE_INSTALL_PREFIX specified, defaulting to /usr")
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "joinx install prefix" FORCE)
endif()


if(DEFINED JOINX_DEBUG_VCF_MERGE)
    message("Vcf merge debugging output enabled")
    add_definitions("-DDEBUG_VCF_MERGE")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-common/cmake")
include(TestHelper)
include(VersionHelper)
include(ProjectHelper)
set(INSTALL_SHARE_PATH "share/joinx${EXE_VERSION_SUFFIX}")
set(INSTALL_DOC_PATH share/doc/joinx${EXE_VERSION_SUFFIX})
message("Install path: ${CMAKE_INSTALL_PREFIX}")
message("Share path: ${INSTALL_SHARE_PATH}")
message("Doc path: ${INSTALL_DOC_PATH}")

configure_file(version/version.h.in version/version.h @ONLY)
include_directories(${PROJECT_BINARY_DIR}/version)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "-std=c99 -Wall -pedantic")
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -ansi -pedantic -std=c++0x -fPIC -Wnon-virtual-dtor")
endif ()

# on linux mint 12, libgtest requires pthreads, so link thread libraries
# everywhere
find_package(Threads)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(BZip2 REQUIRED)
include_directories(BZIP2_INCLUDE_DIR)
add_definitions(${BZIP2_DEFINITIONS})

#set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_SYSTEM_PATHS on)
find_package(Boost 1.51 COMPONENTS program_options filesystem system iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message("Boost include directory: ${Boost_INCLUDE_DIRS}")
message("Boost libraries: ${Boost_LIBRARIES}")

# make sure to pick up headers from library dirs
include_directories("src/lib")

# unit tests
find_package(GTest)
if(GTEST_FOUND)
    message("Google Test framework found, building unit tests")
    enable_testing(true)
    add_projects(test/lib)
else()
    message("Google Test framework not found, no tests will be built")
    message("GTest is available at http://code.google.com/p/googletest/")
    message("Ubuntu users can likely sudo apt-get install libgtest-dev")
endif()

# main project

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_projects(src/lib)
add_projects(src/exe)

# install scripts
add_subdirectory(src/scripts)
# integration testing
add_subdirectory(integration-test)

# generate documentation
add_subdirectory(doc)

include(PackageInfo.cmake)

