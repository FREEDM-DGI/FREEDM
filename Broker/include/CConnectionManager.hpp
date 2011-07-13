//
// ConnectionManager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "CConnection.hpp"

#include <set>
#include <string>
#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "uuid.hpp"

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
    CConnectionManager(freedm::uuid uuid) { std::stringstream ss; ss << uuid; m_uuid = ss.str(); };

    /// Add the specified connection to the manager and start it.
    void Start(ConnectionPtr c);
    
    /// Place a hostname and uuid into the hostname / uuid map.
    void PutHostname(std::string u_, std::string host_);
    
    /// Register a connection with the manager once it has been built.
    void RegisterConnection(std::string uuid, ConnectionPtr c);

    /// Stop the specified connection.
    void Stop(ConnectionPtr c);

    /// Stop all connections.
    void StopAll();

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
    /// Forward map (UUID->Connection)
    connectionmap   m_connections;
    /// Reverse map (Connection->UUID)
    connectionmap_r m_connections_r;
    /// Node UUID
    std::string m_uuid;
    /// Mutex for protecting the handler maps above
    boost::mutex m_Mutex;       
};

} // namespace broker
} // namespace freedm

#endif // CONNECTIONMANAGER_HPP
