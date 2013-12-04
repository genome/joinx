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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build-common/cmake)
include(TestHelper)
include(VersionHelper)
include(ProjectHelper)
include(PlatformChecks)
include(BuildDeps)

set(INSTALL_SHARE_PATH "share/joinx${EXE_VERSION_SUFFIX}")
set(INSTALL_DOC_PATH share/doc/joinx${EXE_VERSION_SUFFIX})
message("Install path: ${CMAKE_INSTALL_PREFIX}")
message("Share path: ${INSTALL_SHARE_PATH}")
message("Doc path: ${INSTALL_DOC_PATH}")

# Dump clang compilation database for use with YCM
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
configure_file(.ycm_extra_conf.py.in ${CMAKE_SOURCE_DIR}/.ycm_extra_conf.py @ONLY)

configure_file(version/version.h.in version/version.h @ONLY)
include_directories(${PROJECT_BINARY_DIR}/version)

if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic -fPIC")
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fPIC ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
else()
    message(
        WARNING
        "Unknown compiler: ${CMAKE_C_COMPILER_ID}, we'll see how this goes..."
    )
endif()


# Find C++11 flags
find_cxx11_flags(CXX11_FLAGS CXX11_FOUND)
if(NOT CXX11_FOUND)
    message(FATAL_ERROR
        "Failed to find C++11 support with compiler:
         ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}).")
endif(NOT CXX11_FOUND)

# Find clock_gettime function
find_library_providing(clock_gettime CLOCK_GETTIME_FOUND CLOCK_GETTIME_LIB rt)
if(NOT CLOCK_GETTIME_FOUND)
    message(FATAL_ERROR "Unable to find the clock_gettime function!")
endif(NOT CLOCK_GETTIME_FOUND)

if (CLOCK_GETTIME_LIB)
    link_libraries(${CLOCK_GETTIME_LIB})
endif (CLOCK_GETTIME_LIB)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")

check_include_file_cxx(cstdint HAVE_CSTDINT)
check_include_file_cxx(stdint.h HAVE_STDINT_H)
# Google Test wants to include <tr1/tuple> instead of just tuple
# If we can't find it, we have to force it to use its own implementation
check_include_file_cxx(tr1/tuple HAVE_TR1_TUPLE)

find_package(Cairomm REQUIRED)
include_directories(${Cairomm_INCLUDE_DIRS})

find_package(BZip2 REQUIRED)
include_directories(BZIP2_INCLUDE_DIR)
add_definitions(${BZIP2_DEFINITIONS})

add_custom_target(deps ALL)

set(REQUIRED_BOOST_LIBS program_options filesystem system iostreams regex)
set(BOOST_URL ${CMAKE_SOURCE_DIR}/vendor/boost-1_55_0-joinx.tar.gz)
set(ZLIB_URL ${CMAKE_SOURCE_DIR}/vendor/zlib-1.2.8.tar.gz)
add_definitions("-DBOOST_CHRONO_HEADER_ONLY")
build_boost(${BOOST_URL} ${CMAKE_BINARY_DIR}/vendor/boost ${REQUIRED_BOOST_LIBS})
build_zlib(${ZLIB_URL} ${CMAKE_BINARY_DIR}/vendor/zlib)

message("-- Boost include directory: ${Boost_INCLUDE_DIRS}")
message("-- Boost libraries: ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

# make sure to pick up headers from library dirs
include_directories("src/lib")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# unit tests
enable_testing(true)
add_projects(test/lib)

# main project

add_projects(src/lib)
add_projects(src/exe)

# install scripts
add_subdirectory(src/scripts)
# integration testing
add_subdirectory(integration-test)

# generate documentation
add_subdirectory(doc)

include(PackageInfo.cmake)
