////////////////////////////////////////////////////////////////////
/// @file      test_cconnection.cpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Unit tests for CConnection class.
///
/// @sa CConnection
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
#include "CDispatcher.hpp"
#include "CMessage.hpp"
#include "IHandler.hpp"

#define BOOST_TEST_MAIN
#include "unit_test.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
using boost::property_tree::ptree;
using namespace boost::property_tree::xml_parser;

static const char * test_xml =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<message><source>00000000-0000-0000-0000-000000000000</source><status>200</status><submessages><test>Test</test><submessage><type>foo</type><value>value1</value></submessage><submessage><type>bar</type><value>value2</value></submessage><submessage><type>baz</type><value>value3</value></submessage></submessages></message>";
static const char * any_xml =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<message><source>00000000-0000-0000-0000-000000000000</source><status>200</status><submessages><submessage><type>foo</type><value>value1</value></submessage><submessage><type>bar</type><value>value2</value></submessage><submessage><type>baz</type><value>value3</value></submessage></submessages></message>";


struct TestHandler : public IReadHandler, IWriteHandler
{
    virtual ~TestHandler()
    { }

    virtual void HandleRead(const ptree &p_tree)
    {
        if( m_throw )
        {
            throw FreedmTestException();
        }
        UNUSED_ARGUMENT( p_tree );
    }

    virtual void HandleWrite( ptree& p_tree )
    {
        if( m_throw )
        {
            throw FreedmTestException();
        }
        UNUSED_ARGUMENT( p_tree );
    }

    bool m_throw;
};

struct TestCDispatcher
{
    TestCDispatcher()
    {
        std::stringstream ss_;
        ss_ << test_xml;

        read_xml(ss_, m_test);
        ss_.clear();
        ss_ << any_xml;
        read_xml(ss_, m_any );

        // Set to 8 for trace output
        Logger::Log::setLevel(0);
    }

    ~TestCDispatcher()
    {

    }

    freedm::broker::CDispatcher m_dispatcher;
    TestHandler m_handler;
    ptree m_test, m_any;
};

BOOST_AUTO_TEST_SUITE( CDispatcherSuite )

BOOST_FIXTURE_TEST_CASE( Construction, TestCDispatcher )
{
    BOOST_CHECK( true );
}

BOOST_FIXTURE_TEST_CASE( RegisterRead, TestCDispatcher )
{
    m_dispatcher.RegisterReadHandler( "test", &m_handler );
}

BOOST_FIXTURE_TEST_CASE( RegisterWrite, TestCDispatcher )
{
    m_dispatcher.RegisterWriteHandler( "any", &m_handler );
}

BOOST_FIXTURE_TEST_CASE( HandleRequestTest, TestCDispatcher )
{
    m_dispatcher.RegisterReadHandler( "test", &m_handler );
    m_handler.m_throw = true;

    BOOST_CHECK_THROW(
        m_dispatcher.HandleRequest( m_test ), FreedmTestException
    );

    BOOST_CHECK_NO_THROW(
        m_dispatcher.HandleRequest( m_any )
    );
}

BOOST_FIXTURE_TEST_CASE( HandleRequestAny, TestCDispatcher )
{
    m_dispatcher.RegisterReadHandler( "any", &m_handler );
    m_handler.m_throw = true;

    BOOST_CHECK_THROW(
        m_dispatcher.HandleRequest( m_test ), FreedmTestException
    );

    BOOST_CHECK_THROW(
        m_dispatcher.HandleRequest( m_any ), FreedmTestException
    );
}

BOOST_FIXTURE_TEST_CASE( HandleWriteTest, TestCDispatcher )
{
    m_dispatcher.RegisterWriteHandler( "test", &m_handler );
    m_handler.m_throw = true;

    BOOST_CHECK_THROW(
        m_dispatcher.HandleWrite( m_test ), FreedmTestException
    );

    BOOST_CHECK_NO_THROW(
        m_dispatcher.HandleWrite( m_any )
    );
}

BOOST_FIXTURE_TEST_CASE( HandleWriteAny, TestCDispatcher )
{
    m_dispatcher.RegisterWriteHandler( "any", &m_handler );
    m_handler.m_throw = true;

    BOOST_CHECK_THROW(
        m_dispatcher.HandleWrite( m_test ), FreedmTestException
    );

    BOOST_CHECK_THROW(
        m_dispatcher.HandleWrite( m_any ), FreedmTestException
    );
}

BOOST_AUTO_TEST_SUITE_END()
