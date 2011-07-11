////////////////////////////////////////////////////////////////////
/// @file      test_crequestparser.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Test suite for CRequestParser class
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
////////////////////////////////////////////////////////////////////

#include "CMessage.hpp"
#include "RequestParser.hpp"
#include "unit_test.hpp"

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace freedm::broker;

static const char * test_xml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<message><source>00000000-0000-0000-0000-000000000000</source><status>200</status><submessages><submessage><type>foo</type><value>value1</value></submessage><submessage><type>bar</type><value>value2</value></submessage><submessage><type>baz</type><value>value3</value></submessage></submessages></message>";

static const char * incomplete_xml =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<message>";

static const char * bad_xml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<message><source>00000000-0000-0000-0000-000000000000</source></message>";

void test_requestparser_bad()
{
    // Parse requires both source and at least an empty
    // submessages tree.
    CMessage message_;
    std::stringstream ss_( bad_xml );
    std::istream_iterator<char> ss_it_( ss_ ),  // Start of stream
        eos_;                                  // End of stream
    boost::tribool result_;
   
    BOOST_CHECK_NO_THROW(
    boost::tie(result_, boost::tuples::ignore) =
            Parse( message_, ss_it_, eos_);
    );

    BOOST_CHECK_EQUAL( result_, false );


}

void test_requestparser_incomplete()
{
    CMessage message_;
    std::stringstream ss_( incomplete_xml );
    std::istream_iterator<char> ss_it_( ss_ ),  // Start of stream
        eos_;                                   // End of stream
    boost::tribool result_;

    BOOST_CHECK_NO_THROW(
    boost::tie(result_, boost::tuples::ignore) =
            Parse( message_, ss_it_, eos_)
    );

    BOOST_CHECK_EQUAL(result_, boost::tribool::indeterminate_value);
}

void test_requestparser_normal()
{
    CMessage message_;
    std::stringstream ss_( test_xml );
    std::istream_iterator<char> ss_it_( ss_ ),  // Start of stream
        eos_;                                  // End of stream
    boost::tribool result_;
   
    BOOST_CHECK_NO_THROW(
    boost::tie(result_, boost::tuples::ignore) =
            Parse( message_, ss_it_, eos_);
    );

    BOOST_CHECK_EQUAL( result_, true );
}

test_suite* init_unit_test_suite( int, char*[] )
{
    // This tests that all headers can pass the compilation step.
    // All headers are included via Server.hpp
    test_suite* test =
            BOOST_TEST_SUITE("broker/RequestParser Tests");

    test->add(BOOST_TEST_CASE(&test_requestparser_normal));
    test->add(BOOST_TEST_CASE(&test_requestparser_incomplete));
    test->add(BOOST_TEST_CASE(&test_requestparser_bad));

    return test;
}


