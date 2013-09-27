cmake_minimum_required(VERSION 2.8)

include(CheckIncludeFileCXX)
include(CheckCXXCompilerFlag)

function(find_cxx11_flag FLAGS)
    list(APPEND CANDIDATE_FLAGS "-std=c++11" "-std=c++0x")

    set(COUNTER 0)
    foreach(elt ${CANDIDATE_FLAGS})
        check_cxx_compiler_flag("${elt}" CXX11_FLAG${COUNTER})
        if (CXX11_FLAG${COUNTER})
            unset(CXX11_FLAG${COUNTER} CACHE)
            set(${FLAGS} ${elt} PARENT_SCOPE)
            break()
        endif()
        math(EXPR COUNTER "${COUNTER} + 1")
    endforeach()
endfunction(find_cxx11_flag FLAGS)

find_cxx11_flag(CXX11_FLAG)
if(CXX11_FLAG)
    set(CMAKE_REQUIRED_FLAGS ${CXX11_FLAG})
    message("C++11 support enabled via ${CXX11_FLAG}")
else()
    message(FATAL_ERROR
        "Unable to determine compiler flag to enable C++11 support!")
endif()

check_include_file_cxx(cstdint HAVE_CSTDINT)
check_include_file_cxx(stdint.h HAVE_STDINT_H)
check_include_file_cxx(tr1/tuple HAVE_TR1_TUPLE)

message("cstdint: ${HAVE_CSTDINT}")
message("stdint.h: ${HAVE_STDINT_H}")
message("tr1/tuple: ${HAVE_TR1_TUPLE}")
