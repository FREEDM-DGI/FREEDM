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
/// Technology, Rolla, /// MO  65409 (ff@mst.edu).
///
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
            CConnectionManager& p_manager, CDispatcher& p_dispatch);

      /// Get the socket associated with the CConnection.
    boost::asio::ip::tcp::socket& GetSocket();

    /// Start the first asynchronous operation for the CConnection.
    void Start();

    /// Stop all asynchronous operations associated with the CConnection.
    void Stop();

    void Send(CMessage p_mesg);

    std::string GetUUID();
    
    void SetUUID(std::string uuid);

    void GiveUUID(std::string uuid);

  private:

    /// Schedules Resend when the timer expires and increases timeout counter.
    void Resend(const boost::system::error_code& error);

    /// Responsible for writing acknowledgements to the stream.
    void HandleSendACK(unsigned int sequenceno);
    
    /// Handle recieving the UUID at the start of a connection
    void HandleReadUUID(const boost::system::error_code &e);
    
    /// Handle reading the header which outlines the length of the incoming
    /// message.
    void HandleReadHeader(const boost::system::error_code &e);

    /// Handle completion of a read operation.
    void HandleRead(const boost::system::error_code& e);

    /// Throws out the buffer on out of order messages.
    void DropMessage(const boost::system::error_code& e);

    /// Handle a send operation posted to the IO thread.
    void HandleSend();

    /// Handle completion of a write operation.
    void HandleWrite(const boost::system::error_code& e);

    /// Tries to parse the headr
    CConnectionHeader ParseHeader();
    
    /// Tests to see if the header is valid(ish)
    bool CanParseHeader();
    
    /// Socket for the CConnection.
    boost::asio::ip::tcp::socket m_socket;

    /// The manager for this CConnection.
    CConnectionManager& m_connManager;

    /// The dispatcher used to process the incoming request.
    CDispatcher& m_dispatch;

    /// Buffer for incoming data.
    boost::array<char, 8192> m_buffer;
    int m_msg_len;

    /// Outgoing message buffer
    // XXX maybe not neccessary? Not sure of the I/O impacts on
    // a single buffer
    boost::array<char, 8192> m_outbuff;

    /// The incoming request.
    CMessage m_message;

    /// Outgoing message Queue
    static const unsigned int WINDOWSIZE = 5;
    typedef std::pair< unsigned int, CMessage > QueueItem;
    SlidingWindow< QueueItem > m_queue;
    boost::asio::deadline_timer m_timeout; 
    unsigned int m_timeouts;
 
    /// The UUID of the remote endpoint for the connection
    std::string m_uuid;

    /// The current sequence number
    unsigned int m_insequenceno;
    unsigned int m_outsequenceno;
    
    /// The Queue of messages waiting for ack.
};

typedef boost::shared_ptr<CConnection> ConnectionPtr;

    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
