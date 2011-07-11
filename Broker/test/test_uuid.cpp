///////////////////////////////////////////////////////////////////////////////
/// @file      <filename>
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   <PROJECT NAME>
///
/// @description <FILE DESCRIPTION> 
///
/// @license
/// These source code files were created at as part of the
/// FREEDM DGI Subthrust, and are
/// intended for use in teaching or research.  They may be 
/// freely copied, modified and redistributed as long
/// as modified versions are clearly marked as such and
/// this notice is not removed.
/// 
/// Neither the authors nor the FREEDM Project nor the
/// National Science Foundation
/// make any warranty, express or implied, nor assumes
/// any legal responsibility for the accuracy,
/// completeness or usefulness of these codes or any
/// information distributed with these codes.
///
/// Suggested modifications or questions about these codes 
/// can be directed to Dr. Bruce McMillin, Department of 
/// Computer Science, Missour University of Science and
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///
///////////////////////////////////////////////////////////////////////////////

#include "uuid.hpp"
#include "unit_test.hpp"

#include <string>
#include <set>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

void test_random_uuid()
{
    freedm::uuid u1, u2;

    BOOST_CHECK( u1 != u2 );
}

test_suite* init_unit_test_suite( int, char*[] )
{
    // This tests that all headers can pass the compilation step.
    // All headers are included via Server.hpp
    test_suite* test = BOOST_TEST_SUITE("UUID Tests");

    test->add(BOOST_TEST_CASE(&test_random_uuid));

    return test;
}



