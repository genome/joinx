cmake_minimum_required(VERSION 2.8)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/joinx.1.gz
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/joinx.pod
    COMMAND pod2man ARGS 
        --section=1
        --release='Joinx ${FULL_VERSION}' 
        --center='Joinx Manual'
        ${CMAKE_CURRENT_SOURCE_DIR}/joinx.pod
        | gzip -9 > ${CMAKE_BINARY_DIR}/joinx.1.gz
)

add_custom_target(doc ALL DEPENDS ${CMAKE_BINARY_DIR}/joinx.1.gz)

install(FILES ${CMAKE_BINARY_DIR}/joinx.1.gz
    DESTINATION share/man/man1
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
)
