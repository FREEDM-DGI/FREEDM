////////////////////////////////////////////////////////////////////////////////
/// @file         CECNHandler.cpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Represents a single CECNHandler from a client.
///
/// These source code files were created at Missouri University of Science and
/// Technology, and are intended for use in teaching or research. They may be
/// freely copied, modified, and redistributed as long as modified versions are
/// clearly marked as such and this notice is not removed. Neither the authors
/// nor Missouri S&T make any warranty, express or implied, nor assume any legal
/// responsibility for the accuracy, completeness, or usefulness of these files
/// or any information distributed with these files.
///
/// Suggested modifications or questions about these files can be directed to
/// Dr. Bruce McMillin, Department of Computer Science, Missouri University of
/// Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
////////////////////////////////////////////////////////////////////////////////

#include "config.hpp"

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/system/error_code.hpp>

using boost::property_tree::ptree;

namespace freedm {
    namespace broker {

namespace {

/// This file's logger.
CLocalLogger Logger(__FILE__);

}

///////////////////////////////////////////////////////////////////////////////
/// CECNHandler::CECNHandler
/// @description Constructor. Creates the socket that will be used to listen
///     for incoming messages.
/// @pre None.
/// @post A socket is created using the Broker's io service.
///////////////////////////////////////////////////////////////////////////////
CECNHandler::CECNHandler()
    : m_socket(CBroker::Instance().GetIOService())
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/// CECNHandler::Instance
/// @description Access the singleton instance of the CECNHandler
///////////////////////////////////////////////////////////////////////////////
CECNHandler& CECNHandler::Instance()
{
    static CECNHandler listener;
    return listener;
}

///////////////////////////////////////////////////////////////////////////////
/// CECNHandler::Start
/// @description Causes the listener to start listening for new messages
/// @pre endpoint is a valid endpoint for the lister to listen on
/// @post The listener is not listening for incoming messages on the socket
///     bound to endpoint
/// @param endpoint the endpoint for the listener to listen on
///////////////////////////////////////////////////////////////////////////////
void CECNHandler::Start(boost::asio::ip::udp::endpoint& endpoint)
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    m_socket.open(endpoint.protocol());
    m_socket.bind(endpoint);
    ScheduleListen();
}

///////////////////////////////////////////////////////////////////////////////
/// CECNHandler::Stop
/// @description Closes the listening socket.
/// @pre None
/// @post The socket used to listen for messages is closed.
///////////////////////////////////////////////////////////////////////////////
void CECNHandler::Stop()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;

    try
    {
        m_socket.close();
    }
    catch (boost::system::system_error& e)
    {
        Logger.Error << "Error calling close: " << e.what() << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CECNHandler::HandleRead
/// @description The callback which accepts messages from the remote sender.
/// @param e The errorcode if any associated.
/// @param bytes_transferred The size of the datagram being read.
/// @pre The connection has had start called and some message has been placed
///   in the buffer by the receive call.
/// @post The ECN message is processed and converted to a DGI style message
///	  and passed to the DGI processes. 
///////////////////////////////////////////////////////////////////////////////
void CECNHandler::HandleRead(const boost::system::error_code& e,
                           std::size_t bytes_transferred)
{
	Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
	const size_t MESSAGE_SIZE = 23;
	if(bytes_transferred == MESSAGE_SIZE)
	{
		std::string magic(m_buffer.begin(), m_buffer.begin()+8);
		if(magic == "ECNDGI00")
		{
			bool type = m_buffer[8];
			int source_addr[4] = {m_buffer[9], m_buffer[10], m_buffer[11], m_buffer[12]}
			int dest_addr[4] = {m_buffer[13], m_buffer[14], m_buffer[15], m_buffer[16]}
			unsigned short source_port = m_buffer[17];
			source_port = (source_port << 8) + m_buffer[18];
			int queue_size = m_buffer[19];
			queue_size = (queue_size << 8) + m_buffer[20];
			queue_size = (queue_size << 8) + m_buffer[21];
			queue_size = (queue_size << 8) + m_buffer[22];
			// Convert to protocol buffer
			ModuleMessage notification;
			EcnHandlingMessage* ecnhm = notification.mutable_ecn_handling_message();
			EcnMessage ecnm* = ecnhm.mutable_ecn_message();
			ecnm->set_type(type);
			std::stringstream ss;
			ss<<source_addr[0]<<"."<<source_addr[1]<<"."<<source_addr[2]<<"."<<source_addr[3];
			ecnm->set_origin_ip(ss.str());
			ss.clear();
			ss<<dest_addr[0]<<"."<<dest_addr[1]<<"."<<dest_addr[2]<<"."<<dest_addr[3];
			ecnm->set_destination_ip(ss.str());
			ss.clear();
			ss<<source_port;
			ecnm->set_destination_port(ss.str());
			ecnm->set_avg_queue_size(queue_size);
			// Send this message into the message queue.
            CDispatcher::Instance().HandleRequest(
                boost::make_shared<const ModuleMessage>(notification),
				CGlobalConfiguration::Instance().GetUUID());

			// Done.
		}		
    }
	ScheduleListen();
}

///////////////////////////////////////////////////////////////////////////////
/// CECNHandler::ScheduleListen
/// @description Makes a call to the Broker's ioservice and requests that the
///     HandleRead function is called when a datagram arrives on the
///     Listener's socket.
/// @pre The m_socket is bound to an endpoint
/// @post HandleRead will be called when a datagram arrives at the socket
///////////////////////////////////////////////////////////////////////////////
void CECNHandler::ScheduleListen()
{
    Logger.Trace << __PRETTY_FUNCTION__ << std::endl;
    Logger.Debug<<"Listening for next message"<<std::endl;
    // We don't care where the messages are coming from, but async_receive_from
    // requires that this variable remain valid until the handler is called.
    m_socket.async_receive_from(
        boost::asio::buffer(m_buffer, CGlobalConfiguration::MAX_PACKET_SIZE),
        m_recv_from, boost::bind(&CECNHandler::HandleRead, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

    } // namespace broker
} // namespace freedm
