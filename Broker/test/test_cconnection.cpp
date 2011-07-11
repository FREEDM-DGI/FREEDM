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
#include "CConnection.hpp"
#include "CConnectionManager.hpp"

#define BOOST_TEST_MAIN
#include "unit_test.hpp"

#include <boost/asio.hpp>

static const char test_xml[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<message><source>00000000-0000-0000-0000-000000000000</source><status>200</status><submessages><test>Test</test><submessage><type>foo</type><value>value1</value></submessage><submessage><type>bar</type><value>value2</value></submessage><submessage><type>baz</type><value>value3</value></submessage></submessages></message>";

void handle_write(const boost::system::error_code& err,
    size_t bytes_transferred, bool* called)
{
  *called = true;
  BOOST_CHECK(!err);
  BOOST_CHECK(bytes_transferred == sizeof(test_xml));
}

using namespace freedm::broker;

struct TestCConnection
{
    TestCConnection() :
        ios(),
        m_conn( new CConnection( ios, cm, d) )
    {
        // Set to 8 for trace output
        Logger::Log::setLevel(8);
    }

    ~TestCConnection()
    {
        m_conn.reset();
    }

    boost::asio::io_service ios;
    CConnectionManager cm;
    CDispatcher d;
    ConnectionPtr m_conn;
};

BOOST_AUTO_TEST_SUITE( CConnectionSuite )

BOOST_FIXTURE_TEST_CASE( Construction, TestCConnection )
{
    BOOST_CHECK( true );
    BOOST_CHECK_EQUAL( false, m_conn->GetSocket().is_open() );
}

BOOST_FIXTURE_TEST_CASE( Connection, TestCConnection )
{
    using namespace boost::asio;

    ip::tcp::acceptor
        acceptor( ios, ip::tcp::endpoint( ip::tcp::v4(), 0));

    ip::tcp::endpoint server_endpoint = acceptor.local_endpoint();
    server_endpoint.address( ip::address_v4::loopback());

    ip::tcp::socket client_socket(ios);

    client_socket.connect( server_endpoint );
    acceptor.accept( m_conn->GetSocket() );

    m_conn->Start();

    bool write_completed = false;
    client_socket.async_write_some(
        boost::asio::buffer(test_xml),
        boost::bind(handle_write,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred,
          &write_completed));

    ios.run();

    BOOST_CHECK(write_completed);

}

BOOST_FIXTURE_TEST_CASE( Send, TestCConnection )
{
    using namespace boost::asio;

    ip::tcp::acceptor
        acceptor( ios, ip::tcp::endpoint( ip::tcp::v4(), 0));

    ip::tcp::endpoint server_endpoint = acceptor.local_endpoint();
    server_endpoint.address( ip::address_v4::loopback());

    ip::tcp::socket server_socket(ios);


    m_conn->GetSocket().connect( server_endpoint );
    acceptor.accept( server_socket );

    m_conn->Start();

    bool write_completed = false;
    server_socket.async_write_some(
        boost::asio::buffer(test_xml),
        boost::bind(handle_write,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred,
          &write_completed));

    ios.run();

    BOOST_CHECK(write_completed);

}


BOOST_AUTO_TEST_SUITE_END()

