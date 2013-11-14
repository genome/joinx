cmake_minimum_required(VERSION 2.6)

# .deb packaging
set(ARCH "i686")
if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(ARCH "x86_64")
endif ()

set(DEFAULT_ETC_ALTERNATIVES_PRIORITY 19)

# The format of the description field is a short summary line followed by a
# longer paragraph indented by a single space on each line
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tool for comparing snv bed files
 Joinx is a tool that operates in genetic data stored
 in bed files (http://genome.ucsc.edu/FAQ/FAQformat.html).
 It can perform set operations (intersection, union,
 difference) as well as produce various types of
 reports (concordance).")
set(CPACK_PACKAGE_NAME "joinx${EXE_VERSION_SUFFIX}")
set(CPACK_PACKAGE_VENDOR "wugc")
set(CPACK_PACKAGE_VERSION ${FULL_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Travis Abbott <tabbott@genome.wustl.edu>")
set(CPACK_SYSTEM_NAME "Linux-${ARCH}")
set(CPACK_TOPLEVEL_TAG "Linux-${ARCH}")
set(CPACK_DEBIAN_PACKAGE_PROVIDES "joinx")
set(CPACK_DEBIAN_PACKAGE_SECTION science)
set(CPACK_DEBIAN_PACKAGE_PRIORITY optional)
set(CPACK_DEBIAN_PACKAGE_REPLACES "snvcmp1.0, snvcmp2.0")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libbz2-1.0 (>= 1.0.5-4ubuntu0.1), libc6 (>= 2.11.1-0ubuntu7.8), libgcc1 (>= 1:4.4.3-4ubuntu5), libstdc++6 (>= 4.4.3-4ubuntu5), zlib1g (>= 1:1.2.3.3.dfsg-15ubuntu1)")

if (CMAKE_BUILD_TYPE MATCHES package)
    set(CPACK_GENERATOR "DEB")
else(CMAKE_BUILD_TYPE MATCHES package)
    set(CPACK_GENERATOR "TGZ")
endif(CMAKE_BUILD_TYPE MATCHES package)

configure_file(debian/postinst.in debian/postinst @ONLY)
configure_file(debian/prerm.in debian/prerm @ONLY)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "debian/postinst;debian/prerm")

include(CPack)
