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
#include "CReliableConnection.hpp"

#include "concurrentqueue.hpp"
#include "SlidingWindow.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>
#include <set>

namespace freedm {
    namespace broker {

class CConnectionManager;

/// Represents a single outgoing connection to a client.
class CConnection
    : public CReliableConnection
{

public:
    /// ConnectionPtr Typedef
    typedef boost::shared_ptr<CConnection> ConnectionPtr;

    /// Construct a CConnection with the given io_service.
    explicit CConnection(boost::asio::io_service& p_ioService,
            CConnectionManager& p_manager, CDispatcher& p_dispatch,
            std::string uuid);

    /// Start the first asynchronous operation for the CConnection.
    void Start();

    /// Stop all asynchronous operations associated with the CConnection.
    void Stop();

    /// Puts a CMessage into the channel.
    void Send(CMessage p_mesg,int max_retries=30);

    /// Handles Notification of an acknowledment being recieved
    void RecieveACK(unsigned int sequenceno);

private:
    /// Remove messages which don't have any retries left from the queue.
    void FlushExpiredMessages();

    /// Has the outgoing connection been synched?
    bool m_synched;

    /// Schedules Resend when the timer expires and increases timeout counter.
    void Resend(const boost::system::error_code& error);

    /// Handles refiring the window.
    void HandleResend();

    /// Sends SYN messages.
    void SendSYN();
    
    /// Handle a send operation posted to the IO thread.
    void HandleSend(CMessage msg, unsigned int sequenceno);

    /// Handle completion of a write operation.
    void HandleWrite(const boost::system::error_code& e);
    
    /// Buffer for incoming data.
    boost::array<char, 8192> m_buffer;
    
    /// The incoming request.
    CMessage m_message;
    
    /// Type for the queued items.
    typedef std::pair<int, CMessage > QueueItem;

    /// The queue of messages
    SlidingWindow< QueueItem > m_queue;

    /// Timer for failed responses.
    boost::asio::deadline_timer m_timeout;

    /// Counter for the number of times the timeout has fired
    /// Without success. 
    unsigned int m_timeouts;
 
    /// The sequence number used for the next outgoing message
    unsigned int m_outsequenceno;
};

typedef boost::shared_ptr<CConnection> ConnectionPtr;

    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
