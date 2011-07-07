cmake_minimum_required(VERSION 2.8)

project(joinx_app)

set(SOURCES 
    Collector.hpp
    CommandBase.cpp
    CommandBase.hpp
    CreateContigsCommand.cpp
    CreateContigsCommand.hpp
    IntersectCommand.cpp
    IntersectCommand.hpp
    JoinX.cpp
    JoinX.hpp
    SnvConcordanceByQualityCommand.cpp
    SnvConcordanceByQualityCommand.hpp
    SnvConcordanceCommand.cpp
    SnvConcordanceCommand.hpp
    SortCommand.cpp
    SortCommand.hpp
    StreamHandler.cpp
    StreamHandler.hpp
    main.cpp
)

set(EXECUTABLE_NAME joinx)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} bedutil fileformats ${Boost_LIBRARIES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES PACKAGE_OUTPUT_NAME ${EXECUTABLE_NAME}${EXE_VERSION_SUFFIX})
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin/)
