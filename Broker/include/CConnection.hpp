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

#include "types/remotehost.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>
#include <set>
#include <deque>

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
    void Send(CMessage p_mesg);

    /// Handles Notification of an acknowledment being recieved
    void RecieveACK(const CMessage &msg);

    /// Handler that calls the correct protocol for accept logic
    bool Recieve(const CMessage &msg);

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
