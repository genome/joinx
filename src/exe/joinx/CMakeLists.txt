cmake_minimum_required(VERSION 2.8)

project(joinx_app)

set(SOURCES
    JoinX.cpp
    JoinX.hpp
    main.cpp
)

set(EXECUTABLE_NAME joinx)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME}
    ui annotate processors reports metrics fileformats io common
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
