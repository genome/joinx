set(TEST_LIBS io ${Boost_LIBRARIES})

add_unit_tests(TestParse
    TestRecord
    TestKvp
)
