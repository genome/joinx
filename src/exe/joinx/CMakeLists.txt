cmake_minimum_required(VERSION 2.6)

project(joinx_app)

set(SOURCES 
    Collector.hpp
    CommandBase.cpp
    CommandBase.hpp
    IntersectCommand.cpp
    IntersectCommand.hpp
    JoinX.cpp
    JoinX.hpp
    SortCommand.cpp
    SortCommand.hpp
    main.cpp
)

set(EXECUTABLE_NAME joinx${EXE_VERSION_SUFFIX})
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil fileformats ${Boost_LIBRARIES})

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
