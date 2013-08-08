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

#include "CGlobalConfiguration.hpp"
#include "CListener.hpp"
#include "IHandler.hpp"
#include "SRemoteHost.hpp"

#include <boost/bimap.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <map>
#include <set>
#include <string>

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

    /// Initialize the connection manager with the uuid from global configuation
    CConnectionManager();

    /// Connection manager teardown.
    ~CConnectionManager() {  };

    /// Add the specified connection to the manager and start it.
    void Start(CListener::ConnectionPtr c);
 
    /// Place a hostname and uuid into the hostname / uuid map.
    void PutHostname(std::string u_, std::string host_, std::string port);
   
    /// Place a hostname and uuid into the hostname / uuid map.
    void PutHostname(std::string u_, SRemoteHost host_);
 
    /// Register a connection with the manager once it has been built.
    void PutConnection(std::string uuid, ConnectionPtr c);

    /// Stop the specified connection.
    void Stop(ConnectionPtr c);
    
    /// Stop the specified connection
    void Stop(CListener::ConnectionPtr c);

    /// Stop all connections.
    void StopAll();

    /// Handle Rounds
    void ChangePhase(bool newround);

    /// Get The UUID
    std::string GetUUID() { return m_uuid; };

    /// Get The Hostname
    SRemoteHost GetHostname() { return m_hostname; };
    
    /// Get the hostname from the UUID.
    SRemoteHost GetHostnameByUUID( std::string uuid ) const; 

    /// Fetch a connection pointer via UUID
    ConnectionPtr GetConnectionByUUID( std::string uuid_ );

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
    
    // Transient Network Simulation
    /// Load a network configuration & apply it.
    void LoadNetworkConfig();

private:
    /// Mapping from uuid to hostname.
    hostnamemap m_hostnames;
    /// Hostname of this node.
    SRemoteHost m_hostname;
    /// Forward map (UUID->Connection)
    connectionmap   m_connections;
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
