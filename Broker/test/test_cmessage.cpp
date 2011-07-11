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

#include "CMessage.hpp"
#include "unit_test.hpp"

#include <string>
#include <sstream>
#include <set>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

static const char * test_xml = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<message><source>00000000-0000-0000-0000-000000000000</source><status>200</status><submessages><submessage><type>foo</type><value>value1</value></submessage><submessage><type>bar</type><value>value2</value></submessage><submessage><type>baz</type><value>value3</value></submessage></submessages></message>";

static  const char * submesg[][2] = {
        { "foo", "value1" },
        { "bar", "value2" },
        { "baz", "value3" }
};

void test_message_save()
{
    freedm::broker::CMessage m;
    std::stringstream ss;

    ptree child_;
    // Populate message with data
    m.m_srcUUID = "00000000-0000-0000-0000-000000000000";
   
    foreach( const char ** str, submesg )
    {
        child_.add("type", str[0] );
        child_.add("value", str[1] );
        m.m_submessages.add_child( "submessage", child_ );
        child_.clear();
    }
    m.m_status = freedm::broker::CMessage::OK;

    // Save the message
    m.Save( ss );

    BOOST_CHECK_EQUAL( std::string( test_xml ), ss.str() );
}

void test_message_load()
{
    freedm::broker::CMessage m;
    std::stringstream ss( test_xml );

    ptree child_, submessages_;

    foreach( const char ** str, submesg )
    {
        child_.add("type", str[0] );
        child_.add("value", str[1] );
        submessages_.add_child( "submessage", child_ );
        child_.clear();
    }

    m.Load( ss );

    BOOST_CHECK_EQUAL(
        std::string( "00000000-0000-0000-0000-000000000000"),
        m.m_srcUUID );
    BOOST_CHECK_EQUAL( freedm::broker::CMessage::OK, m.m_status );
    BOOST_CHECK_MESSAGE( m.m_submessages == submessages_,
        "ptree check between m and submessages_ failed." );

}

void test_message_save_load()
{
    freedm::broker::CMessage m1, m2;
    std::stringstream ss;
    ptree child_, submessages_;

    // Static data
    foreach( const char ** str, submesg )
    {
        child_.add("type", str[0] );
        child_.add("value", str[1] );
        submessages_.add_child( "submessage", child_ );
        child_.clear();
    }

    // Populate message with data
    m1.m_srcUUID = "00000000-0000-0000-0000-000000000000";

    foreach( const char ** str, submesg )
    {
        child_.add("type", str[0] );
        child_.add("value", str[1] );
        m1.m_submessages.add_child( "submessage", child_ );
        child_.clear();
    }

    // Save the message
    m1.Save( ss );

    BOOST_CHECK_EQUAL( ss.str(), test_xml );

    // Load it into the other message
    m2.Load( ss );

    // Check against static values
    BOOST_CHECK_EQUAL( std::string( "00000000-0000-0000-0000-000000000000"), m2.m_srcUUID );
    BOOST_CHECK_MESSAGE( m2.m_submessages == submessages_, "ptree static checks failed.");
    BOOST_CHECK_EQUAL( freedm::broker::CMessage::OK, m2.m_status );

    // Check against other values
    BOOST_CHECK_EQUAL( m1.m_srcUUID, m2.m_srcUUID );
    BOOST_CHECK_MESSAGE( m1.m_submessages == m2.m_submessages, "ptree check between m1 and m2 failed." );
    BOOST_CHECK_EQUAL( m1.m_status, m2.m_status );


}

test_suite* init_unit_test_suite( int, char*[] )
{
    Logger::Log::setLevel( 8 );
    // This tests that all headers can pass the compilation step.
    // All headers are included via Server.hpp
    test_suite* test = BOOST_TEST_SUITE("broker/CMessage Tests");

    test->add(BOOST_TEST_CASE(&test_message_save));
    test->add(BOOST_TEST_CASE(&test_message_load));
    test->add(BOOST_TEST_CASE(&test_message_save_load));

    return test;
}



