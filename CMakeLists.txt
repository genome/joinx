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

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_SOURCE_DIR}/build-common/cmake")
include(TestHelper)
include(VersionHelper)
include(ProjectHelper)
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fPIC ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
    message("Set cflags to ${CMAKE_C_FLAGS}")
    message("Set cxxflags to ${CMAKE_CXX_FLAGS}")
else()
    message(
        WARNING
        "Unknown compiler: ${CMAKE_C_COMPILER_ID}, we'll see how this goes..."
    )
endif()

include(PlatformChecks)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

find_package(BZip2 REQUIRED)
include_directories(BZIP2_INCLUDE_DIR)
add_definitions(${BZIP2_DEFINITIONS})

set(REQUIRED_BOOST_LIBS program_options filesystem system iostreams)
find_package(Boost 1.51 COMPONENTS ${REQUIRED_BOOST_LIBS})
if (Boost_FOUND)
else()
    message("Failed to find boost, we'll try to download and build it")
    include(BuildBoost)
endif()

include_directories(${Boost_INCLUDE_DIRS})
message("Boost include directory: ${Boost_INCLUDE_DIRS}")
message("Boost libraries: ${Boost_LIBRARIES}")

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
