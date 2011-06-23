macro(def_integration_test testName script)
    add_test(
        NAME ${testName}
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/${script} $<TARGET_FILE:joinx>
    )
    set_tests_properties(${testName} PROPERTIES LABELS integration)
endmacro(def_integration_test testName script)

def_integration_test(Intersect intersect.t)
def_integration_test(Sort sort.t)
def_integration_test(CreateContigs create-contigs.t)
