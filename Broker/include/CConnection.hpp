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
#include "CMessage.hpp"
#include "CReliableConnection.hpp"
#include "SRemoteHost.hpp"

#include <deque>
#include <iomanip>
#include <set>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace freedm {
    namespace broker {

class IProtocol;

/// Represents a single outgoing connection to a client.
class CConnection
    : public CReliableConnection
{

public:
    /// ConnectionPtr Typedef
    typedef boost::shared_ptr<CConnection> ConnectionPtr;

    /// Construct a CConnection with the given io_service.
    explicit CConnection(boost::asio::io_service& p_ioService,
            CConnectionManager& p_manager, CBroker& p_broker,
            std::string uuid);

    /// Start the first asynchronous operation for the CConnection.
    void Start();

    /// Stop all asynchronous operations associated with the CConnection.
    void Stop();

    /// Puts a CMessage into the channel.
    void Send(CMessage & p_mesg);

    /// Handles Notification of an acknowledment being recieved
    void RecieveACK(const CMessage &msg);

    /// Handler that calls the correct protocol for accept logic
    bool Recieve(const CMessage &msg);

    /// Change Phase Event
    void ChangePhase(bool newround);
private:
    typedef boost::shared_ptr<IProtocol> ProtocolPtr;
    typedef std::map<std::string,ProtocolPtr> ProtocolMap;
    /// Protocol Handler Map
    ProtocolMap m_protocols;
    
    /// Default protocol
    std::string m_defaultprotocol;
};

typedef boost::shared_ptr<CConnection> ConnectionPtr;

    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
