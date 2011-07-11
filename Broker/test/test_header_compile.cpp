#include "CBroker.hpp"

#include "unit_test.hpp"

test_suite* init_unit_test_suite( int, char*[] )
{
    // This tests that all headers can pass the compilation step.
    // All headers are included via Server.hpp
    test_suite* test = BOOST_TEST_SUITE("broker/header_compiler_test");
    test->add(BOOST_TEST_CASE(&null_test));
    return test;
}
