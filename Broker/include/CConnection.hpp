////////////////////////////////////////////////////////////////////
/// @file      CConnection.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>, Stephen Jackson <scj7t4@mst.edu>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Declare CConnection class
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
/// Technology, Rolla, MO 65409 (ff@mst.edu).
////////////////////////////////////////////////////////////////////

#ifndef CCONNECTION_HPP
#define CCONNECTION_HPP

#include "CMessage.hpp"
#include "CDispatcher.hpp"
#include "CConnectionHeader.hpp"

#include "concurrentqueue.hpp"
#include "SlidingWindow.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>

namespace freedm {
    namespace broker {

const int UUIDLENGTH = 36;

class CConnectionManager;

/// Represents a single CConnection from a client.
class CConnection
    : public boost::enable_shared_from_this<CConnection>,
      private boost::noncopyable
{

public:
    /// Construct a CConnection with the given io_service.
    explicit CConnection(boost::asio::io_service& p_ioService,
            CConnectionManager& p_manager, CDispatcher& p_dispatch,
            std::string uuid);

    /// Get the socket associated with the CConnection.
    boost::asio::ip::udp::socket& GetSocket();

    /// Start the first asynchronous operation for the CConnection.
    void Start();

    /// Stop all asynchronous operations associated with the CConnection.
    void Stop();

    /// Puts a CMessage into the channel.
    void Send(CMessage p_mesg,bool sequence=true);

    /// Get Remote UUID
    std::string GetUUID() { return m_uuid; };

private:
    /// Schedules Resend when the timer expires and increases timeout counter.
    void Resend(const boost::system::error_code& error);

    /// Handles refiring the window.
    void HandleResend();

    /// Responsible for writing acknowledgements to the stream.
    void SendACK(std::string uuid, std:: string hostname, unsigned int sequenceno);

    /// Handles Notification of an acknowledment being recieved
    void RecieveACK(unsigned int sequenceno);
    
    /// Handle completion of a read operation.
    void HandleRead(const boost::system::error_code& e, std::size_t bytes_transferred);

    /// Handle a send operation posted to the IO thread.
    void HandleSend(CMessage msg);

    /// Handle completion of a write operation.
    void HandleWrite(const boost::system::error_code& e);
    
    /// Socket for the CConnection.
    boost::asio::ip::udp::socket m_socket;
    
    /// Variable used for tracking the remote endpoint of incoming messages.
    boost::asio::ip::udp::endpoint m_endpoint;

    /// The manager for this CConnection.
    CConnectionManager& m_connManager;

    /// The dispatcher used to process the incoming request.
    CDispatcher& m_dispatch;

    /// Buffer for incoming data.
    boost::array<char, 8192> m_buffer;
    
    /// The incoming request.
    CMessage m_message;

    /// Outgoing message Queue, window size
    static const unsigned int WINDOWSIZE = 5;
    typedef std::pair< unsigned int, CMessage > QueueItem;
    /// The queue of messages
    SlidingWindow< QueueItem > m_queue;
    /// Timer for failed responses.
    boost::asio::deadline_timer m_timeout;
    /// Counter for the number of times the timeout has fired
    /// Without success. 
    unsigned int m_timeouts;
 
    /// The UUID of the remote endpoint for the connection
    std::string m_uuid;

    /// The current sequence number (incoming channel only)
    std::map<std::string,unsigned int> m_insequenceno;
    
    /// The sequence number used for the next outgoing message
    unsigned int m_outsequenceno;
    
};

typedef boost::shared_ptr<CConnection> ConnectionPtr;

    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
