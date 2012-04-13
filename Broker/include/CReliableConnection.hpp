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

#ifndef CRELIABLECONNECTION_HPP
#define CRELIABLECONNECTION_HPP

#include "CMessage.hpp"
#include "CBroker.hpp"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <iomanip>

namespace freedm {
    namespace broker {

class CConnectionManager;
class CDispatcher;
class CBroker;

/// Represents a single connection to from a client.
class CReliableConnection
    : public boost::enable_shared_from_this<CReliableConnection>,
      public boost::noncopyable
{
public:
    /// Typedef for the connection pointer
    typedef boost::shared_ptr<CReliableConnection> ConnectionPtr;

    /// Construct a CConnection with the given io_service.
    explicit CReliableConnection(boost::asio::io_service& p_ioService,
            CConnectionManager& p_manager, CBroker& p_broker,
            std::string uuid);

    /// Get the socket associated with the CConnection.
    boost::asio::ip::udp::socket& GetSocket();

    /// Start the first asynchronous operation for the CConnection.
    virtual void Start() = 0;

    /// Stop all asynchronous operations associated with the CConnection.
    virtual void Stop() = 0;

    /// Get associated UUID
    std::string GetUUID();

    /// Get Connection Manager
    CConnectionManager& GetConnectionManager();

    /// Get the broker
    CBroker& GetBroker();

    /// Get the dispatcher
    CDispatcher& GetDispatcher();
    
    /// Get the ioservice
    boost::asio::io_service& GetIOService();
    
    /// Set the connection reliability for DCUSTOMNETWORK
    void SetReliability(int r);
    
    /// Get the connection reliability for DCUSTOMNETWORK
    int GetReliability();

    /// The maximum packet size in bytes
    static const unsigned int MAX_PACKET_SIZE = 60000;
private:
    /// Socket for the CConnection.
    boost::asio::ip::udp::socket m_socket;

    /// The manager for this CConnection.
    CConnectionManager& m_connManager;

    /// The dispatcher used to process the incoming request.
    CBroker& m_broker;
 
    /// The UUID of the remote endpoint for the connection
    std::string m_uuid;

    /// The reliability of the connection (FOR -DCUSTOMNETWORK)
    int m_reliability;
};


    } // namespace broker
} // namespace freedm

#endif // CCONNECTION_HPP
