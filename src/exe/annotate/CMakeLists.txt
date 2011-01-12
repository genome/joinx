cmake_minimum_required(VERSION 2.6)

project(annotate)

set(SOURCES 
    AnnotateApp.cpp
    AnnotateApp.hpp
    main.cpp
)

set(EXECUTABLE_NAME annotate${VERSION_MAJOR}.${VERSION_MINOR})
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil annotate ${Boost_LIBRARIES})

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
