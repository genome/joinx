macro(def_integration_test testName script)
    add_test(
        NAME ${testName}
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/${script} $<TARGET_FILE:joinx>
    )
    set_tests_properties(${testName} PROPERTIES LABELS integration)
endmacro(def_integration_test testName script)

def_integration_test(CreateContigs test_create_contigs.py)
def_integration_test(Intersect test_intersect.py)
def_integration_test(SnvConcordance test_snv_concordance.py)
def_integration_test(Sort test_sort.py)
def_integration_test(VcfMerge test_vcf_merge.py)
