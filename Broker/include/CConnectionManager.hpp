////////////////////////////////////////////////////////////////////
/// @file      CConnection.hpp
///
/// @author    Derek Ditch <derek.ditch@mst.edu>
///            Stephen Jackson <scj7t4@mst.edu>
///            Christopher M. Kohloff <chris@kohloff.com>
///
/// @compiler  C++
///
/// @project   FREEDM DGI
///
/// @description Connection manager, for the connections.
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
#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "CConnection.hpp"
#include "CListener.hpp"
#include "CReliableConnection.hpp"

#include <set>
#include <string>
#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "uuid.hpp"
#include "IHandler.hpp"

namespace freedm {
namespace broker {

/// Manages open connections so that they may be cleanly stopped
class CConnectionManager
    : private boost::noncopyable
{
public:
    /// Typedef for the map which handles uuid to hostname
    typedef std::map<std::string, std::string> hostnamemap;
    
    /// Typedef for the map which handles uuid to connection 
    typedef std::map<std::string, ConnectionPtr> connectionmap;

    /// Typedef for the map which handles connection to uuid.
    typedef std::map<ConnectionPtr, std::string> connectionmap_r;

    /// Initialize the connection manager with the node uuid.
    CConnectionManager(freedm::uuid uuid, std::string hostname);

    /// Connection manager teardown.
    ~CConnectionManager() {  };

    /// Add the specified connection to the manager and start it.
    void Start(CListener::ConnectionPtr c);
 
    /// Place a hostname and uuid into the hostname / uuid map.
    void PutHostname(std::string u_, std::string host_);
    
    /// Register a connection with the manager once it has been built.
    void PutConnection(std::string uuid, ConnectionPtr c);

    /// Stop the specified connection.
    void Stop(CConnection::ConnectionPtr c);
    
    /// Stop the specified connection
    void Stop(CListener::ConnectionPtr c);

    /// Stop all connections.
    void StopAll();

    /// Get The UUID
    std::string GetUUID() { return m_uuid; };

    /// Get The Hostname
    std::string GetHostname() { return m_hostname; };
    
    /// Get the hostname from the UUID.
    std::string GetHostnameByUUID( std::string uuid ) const; 

    /// Fetch a connection pointer via UUID
    ConnectionPtr GetConnectionByUUID( std::string uuid_,  boost::asio::io_service& ios,  CDispatcher &dispatch_ );

    /// An iterator to the beginning of the hostname map
    hostnamemap::iterator GetHostnamesBegin() { return m_hostnames.begin(); };

    /// An iterator to the end of the hostname map.
    hostnamemap::iterator GetHostnamesEnd() { return m_hostnames.end(); };

    /// An iterator to the specified hostname.
    hostnamemap::iterator GetHostname(std::string uuid) { return m_hostnames.find(uuid); };

    /// Iterator to the beginning of the connection map.
    connectionmap::iterator GetConnectionsBegin() { return m_connections.begin(); };

    /// Iterator to the end of the connections map.
    connectionmap::iterator GetConnectionsEnd() { return m_connections.end(); };

private:
    /// Mapping from uuid to hostname.
    hostnamemap m_hostnames;
    /// Hostname of this node.
    std::string m_hostname;
    /// Forward map (UUID->Connection)
    connectionmap   m_connections;
    /// Reverse map (Connection->UUID)
    connectionmap_r m_connections_r;
    /// Incoming messages channel
    CListener::ConnectionPtr m_inchannel;
    /// Node UUID
    std::string m_uuid;
    /// Mutex for protecting the handler maps above
    boost::mutex m_Mutex;       
};

} // namespace broker
} // namespace freedm

#endif // CONNECTIONMANAGER_HPP
