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

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class CConnectionManager
    : private boost::noncopyable
{
public:
    typedef std::map<std::string, std::string> hostnamemap;
    typedef std::map<std::string, ConnectionPtr> connectionmap;
    typedef std::map<ConnectionPtr, std::string> connectionmap_r;
    /// Initialize the connection manager with the node uuid.
    CConnectionManager(freedm::uuid uuid) { std::stringstream ss; ss << uuid; m_uuid = ss.str(); };

    /// Add the specified connection to the manager and start it.
    void Start(ConnectionPtr c);

    //void Put(ConnectionPtr c, freedm::uuid u_ );
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

    /// Get iterators to the hostname map.
    hostnamemap::iterator GetHostnamesBegin() { return m_hostnames.begin(); };
    hostnamemap::iterator GetHostnamesEnd() { return m_hostnames.end(); };
    hostnamemap::iterator GetHostname(std::string uuid) { return m_hostnames.find(uuid); };

    /// Get iterators to the forward hostname map.
    connectionmap::iterator GetConnectionsBegin() { return m_connections.begin(); };
    connectionmap::iterator GetConnectionsEnd() { return m_connections.end(); };

private:
    /// Mapping from uuid to hostname.
    hostnamemap m_hostnames;
    /// The managed connections.
    connectionmap   m_connections; // Forward Mapping (UUID->connection)
    connectionmap_r m_connections_r; // Reverse Mapping (connection->UUID)
    /// Node UUID
    std::string m_uuid;
 
 
 // Mutexes for protecting the handler maps above
    boost::mutex m_Mutex;       
};

} // namespace broker
} // namespace freedm

#endif // CONNECTIONMANAGER_HPP
