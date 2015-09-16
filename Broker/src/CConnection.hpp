////////////////////////////////////////////////////////////////////////////////
/// @file         CConnection.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
///
/// @project      FREEDM DGI
///
/// @description  Declare CConnection class
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

#ifndef CCONNECTION_HPP
#define CCONNECTION_HPP

#include "CDispatcher.hpp"
#include "SRemoteHost.hpp"
#include "IProtocol.hpp"

#include <memory>
#include <deque>

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>

namespace google {
  namespace protobuf {

class Message;

  }
}

namespace freedm {
    namespace broker {


/// Used for errors communicating with peers.
struct EConnectionError
    : virtual std::runtime_error
{
    explicit EConnectionError(const std::string& what)
        : std::runtime_error(what) { }
};

/// Represents a single outgoing connection to a client.
class CConnection
    : public IProtocol
{

public:
    typedef boost::shared_ptr<CConnection> ConnectionPtr;

    /// Construct a CConnection to a peer
    CConnection(std::string uuid, boost::asio::ip::udp::endpoint endpoint);

    /// Destructor
    ~CConnection();

    /// Handles Stopping the timers etc
    virtual void Stop();

    /// Puts a message into the channel.
    void Send(const ModuleMessage& msg);

    /// Handles acknowledgement messages from the peer.
    void ReceiveACK(const ProtocolMessage& msg);

    /// Handles messages from the peer.
    bool Receive(const ProtocolMessage& msg);

    /// Performs an action based on receiving a Protocol Message Window.
    void OnReceive();
   
    /// Handles Writing an ack for the input message to the channel
    void SendACK(const ProtocolMessage& msg);
    
    /// Sends a synchronizer
    void SendSYN();
 
    /// Handles writing the message to the underlying connection
    void Write(ProtocolMessageWindow & msg);
    /// Writes a whole window to the channel
    void WriteWindow();
private:
    /// Resend outstanding messages
    void Resend(const boost::system::error_code& err);
    /// Timeout for resends
    boost::asio::deadline_timer m_timeout;
    /// The expected next in sequence number
    unsigned int m_inseq;
    /// The next number to assign to an outgoing message
    unsigned int m_outseq;
    /// Marks if this has been synced.
    bool m_insync;
    /// Counts the number of times this one has been resynced
    unsigned int m_inresyncs;
    /// Time the last accepted sync was
    boost::posix_time::ptime m_insynctime;
    /// Marks if we've sent the outsync for this connection
    bool m_outsync;
    /// Keeps track of the last resync that we've seen
    google::protobuf::uint64 m_outsynchash;
    /// Marks if we should send the kill hash.
    bool m_sendkills;
    /// The hash to... MURDER.
    unsigned int m_sendkill;
    /// The window
    std::deque<ProtocolMessage> m_window;
    std::deque<ProtocolMessage> m_ack_window;
    /// Sequence modulo
    static const unsigned int SEQUENCE_MODULO = 1024;
    /// Refire time in MS
    static const unsigned int REFIRE_TIME = 10;
    /// The number of messages that have to be dropped before the connection is dead
    static const unsigned int MAX_DROPPED_MSGS = 3;
    /// The number that have been dropped.
    unsigned int m_dropped;
    /// Indicates if the timer is active.
    bool m_timer_active;
};

typedef boost::shared_ptr<CConnection> ConnectionPtr;

    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
