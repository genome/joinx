macro(def_integration_test testName script)
    add_test(
        NAME ${testName}
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/${script} $<TARGET_FILE:joinx>
    )
    set_tests_properties(${testName} PROPERTIES LABELS integration)
endmacro(def_integration_test testName script)

add_subdirectory(intersect)
add_subdirectory(sort)
