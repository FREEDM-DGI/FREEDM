////////////////////////////////////////////////////////////////////////////////
/// @file         CConnectionManager.hpp
///
/// @author       Derek Ditch <derek.ditch@mst.edu>
/// @author       Stephen Jackson <scj7t4@mst.edu>
/// @author       Christopher M. Kohloff <chris@kohloff.com>
///
/// @project      FREEDM DGI
///
/// @description  Connection manager, for the connections.
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

#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "SRemoteHost.hpp"

#include <map>
#include <set>
#include <string>

#include <boost/bimap.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>

namespace freedm {
namespace broker {

class CConnection;

/// Manages open connections so that they may be cleanly stopped
class CConnectionManager
    : private boost::noncopyable
{
public:
    /// ConnectionPtr Typedef
    typedef boost::shared_ptr<CConnection> ConnectionPtr;

    /// Typedef for the map which handles uuid to hostname
    typedef std::map<std::string, SRemoteHost> hostnamemap;

    /// Typedef for the map which handles uuid to connection
    typedef boost::bimap<std::string, ConnectionPtr> connectionmap;

    /// Access the singleton instance of the connection manager
    static CConnectionManager& Instance();

    /// Place a host/port and uuid into the host / uuid map.
    void PutHost(std::string u, std::string host, std::string port);

    /// Place a host/port and uuid into the host / uuid map.
    void PutHost(std::string u, SRemoteHost host);

    /// Register a connection with the manager once it has been built.
    void PutConnection(std::string uuid, ConnectionPtr c);

    /// Stop the specified connection.
    void Stop(ConnectionPtr c);

    /// Stop all connections.
    void StopAll();

    /// Handle changing rounds.
    void ChangePhase(bool newround);

    /// Fetch a connection pointer via UUID
    ConnectionPtr GetConnectionByUUID( std::string uuid );

    /// Creates a connection by binding it to an endpoint
    ConnectionPtr CreateConnection(std::string uuid, boost::asio::ip::udp::endpoint endpoint);
    
    /// Returns true if this map is currently tracking a connection to this peer.
    bool HasConnection(std::string uuid);

    /// An iterator to the beginning of the hostname map.
    hostnamemap::iterator GetHostsBegin() { return m_hosts.begin(); };

    /// An iterator to the end of the hostname map.
    hostnamemap::iterator GetHostsEnd() { return m_hosts.end(); };

    /// An iterator to the specified hostname.
    hostnamemap::iterator GetHost(std::string uuid) { return m_hosts.find(uuid); };

    /// Iterator to the beginning of the connection map.
    connectionmap::iterator GetConnectionsBegin() { return m_connections.begin(); };

    /// Iterator to the end of the connections map.
    connectionmap::iterator GetConnectionsEnd() { return m_connections.end(); };

    // Transient Network Simulation
    /// Load a network configuration & apply it.
    void LoadNetworkConfig();

private:
    /// Private constructor for the singleton instance
    CConnectionManager();
    /// Mapping from uuid to host.
    hostnamemap m_hosts;
    /// Forward map (UUID->Connection)
    connectionmap m_connections;
    /// Mutex for protecting the handler maps above
    boost::mutex m_Mutex;
};

} // namespace broker
} // namespace freedm

#endif // CONNECTIONMANAGER_HPP
