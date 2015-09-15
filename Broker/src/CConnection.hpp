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

#include "CProtocolSR.hpp"
#include "CDispatcher.hpp"
#include "SRemoteHost.hpp"

#include <memory>

#include <boost/noncopyable.hpp>

namespace google {
  namespace protobuf {

class Message;

  }
}

namespace freedm {
    namespace broker {

class IProtocol;

/// Used for errors communicating with peers.
struct EConnectionError
    : virtual std::runtime_error
{
    explicit EConnectionError(const std::string& what)
        : std::runtime_error(what) { }
};

/// Represents a single outgoing connection to a client.
class CConnection
    : private boost::noncopyable
{

public:
    typedef boost::shared_ptr<CConnection> ConnectionPtr;

    /// Construct a CConnection to a peer
    CConnection(std::string uuid, boost::asio::ip::udp::endpoint endpoint);

    /// Destructor
    ~CConnection();

    /// Stop all asynchronous operations associated with the CConnection.
    void Stop();

    /// Tests to see if the protocol is stopped
    bool GetStopped();

    /// Puts a message into the channel.
    void Send(const ModuleMessage& msg);

    /// Handles acknowledgement messages from the peer.
    void ReceiveACK(const ProtocolMessage& msg);

    /// Handles messages from the peer.
    bool Receive(const ProtocolMessage& msg);

    /// Performs an action based on receiving a Protocol Message Window.
    void OnReceive();
    
    /// Allows protocols to peform an action when a phase ends.
    void ChangePhase(bool newround);

    /// Gets the UUID of the peer for this connection.
    std::string GetUUID() const;

    /// Set the connection reliability for DCUSTOMNETWORK
    void SetReliability(int r);
    
    /// Get the connection reliability for DCUSTOMNETWORK
    int GetReliability() const;
private:

    /// The network protocol to use for sending/receiving messages
    boost::shared_ptr<IProtocol> m_protocol;
};

typedef boost::shared_ptr<CConnection> ConnectionPtr;

    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
